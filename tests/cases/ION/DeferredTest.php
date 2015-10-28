<?php

namespace ION;

use ION\Test\TestCase;

class DeferredTest extends TestCase {
    /**
     * @memcheck
     */
    public function testResolve() {
        $defer = new Deferred(function () {
            $this->data["cancel"] = true;
        });
        $defer->then(function ($data, $error = null) {
            $this->data["result"] = $data;
            $this->data["error"]  = $error;
        });
        $defer->done("some result");
        $this->assertEquals([
            "result" => "some result",
            "error" => null
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testResolveException() {
        $defer = new Deferred(function () {
            $this->data["cancel"] = true;
        });
        $defer->then(function ($data, $error = null) {
            $this->data["result"] = $data;
            $this->data["error"] = $error;
            throw new \RuntimeException("Test exception in then()");
        });
        $defer->done("some result");
        $this->assertEquals([
            "result" => "some result",
            "error" => null
        ], $this->data);
    }


    /**
     * @memcheck
     */
    public function testCancel() {
        $defer = new Deferred(function ($reason) {
            $this->data["cancel"] = $this->exception2array($reason);
        });
        $defer->then(function ($data, $error) {
            $this->data["result"] = $data;
            $this->data["error"]  = $this->exception2array($error);
        });
        $defer->cancel("just reject");
        $this->assertEquals([
            "result" => null,
            "cancel" => [
                "exception" => 'ION\Promise\CancelException',
                "message" => "just reject",
                "code" => 0
            ],
            "error" => null
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testCancelException() {
        $defer = new Deferred(function ($reason) {
            $this->data["cancel"] = $this->exception2array($reason);
            throw new \RuntimeException("Test exception in cancel()");
        });
        $defer->then(function ($data, $error) {
            $this->data["result"] = $data;
            $this->data["error"]  = $this->exception2array($error);
        });
        $defer->cancel("just reject");
        $this->assertEquals([
            "cancel" => [
                "exception" => 'ION\Promise\CancelException',
                "message" => "just reject",
                "code" => 0
            ],
            "result" => null,
            "error" => [
                "exception" => 'RuntimeException',
                "message" => "Test exception in cancel()",
                "code" => 0
            ]
        ], $this->data);
    }

    /**
     *
     * @memcheck
     */
    public function testReject() {
        $defer = new Deferred(function ($reason) {
            $this->data["cancel"] = $this->exception2array($reason);
        });
        $defer->then(function ($data, $error) {
            $this->data["result"] = $data;
            $this->data["error"]  = $this->exception2array($error);
        });
        $defer->fail(new \RuntimeException("this is fail"));
        $this->assertEquals([
            "result" => null,
            "error" => [
                "exception" => 'RuntimeException',
                "message" => "this is fail",
                "code" => 0
            ]
        ], $this->data);
    }

    /**
     * @expectedException \ION\InvalidUsageException
     * @expectedExceptionMessage Deferred has been finished
     * @memcheck
     */
    public function testRejectAfterDone() {
        $defer = new Deferred(function ($reason) {
        });
        $defer->then(function ($data, $error) {
        });
        $defer->done("some result");
        $defer->cancel("bad reason");
    }

    /**
     * @expectedException \ION\InvalidUsageException
     * @expectedExceptionMessage Deferred has been finished
     * @memcheck
     */
    public function testRejectAfterFail() {
        $defer = new Deferred(function () { });
        $defer->then(function () { });
        $defer->fail(new \RuntimeException("errorka"));
        $defer->cancel("bad reason");
    }
}