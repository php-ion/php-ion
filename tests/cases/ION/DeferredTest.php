<?php

namespace ION;

use ION\Test\TestCase;

class DeferredTest extends TestCase {
    public $should_fail = false;
    public $should_success = false;


    public function setUp() {
        parent::setUp();
        $this->should_fail = false;
        $this->should_success = false;
    }

    /**
     * @group testResolve
     * @memcheck
     */
    public function testResolve() {
        $defer = new Deferred(function () {
            $this->should_fail = true;
        });
        $defer->then(function ($data, $error = null) {
            $this->should_success = true;
            $this->assertNull($error);
            $this->assertSame($data, "some result");
        });
        $defer->done("some result");
        $this->assertFalse($this->should_fail);
        $this->assertTrue($this->should_success);
    }

    /**
     * @group testResolveException
     * @expectedException \RuntimeException
     * @expectedExceptionMessage Test exception in then()
     * @memcheck
     */
    public function testResolveException() {
        $defer = new Deferred(function () {
            $this->fail("deferred already done");
        });
        $defer->then(function ($data, $error) {
            throw new \RuntimeException("Test exception in then()");
        });
        $defer->done("some result");
        $this->fail("should be exception");
    }


    /**
     * @group testReject
     * @memcheck
     */
    public function testReject() {
        $defer = new Deferred(function ($reason) {
            $this->should_success = true;
            $this->assertException($reason, "just reject", 0);
        });
        $defer->then(function ($data, $error) {
            $this->should_fail = true;
        });
        $defer->cancel("just reject");
        $this->assertFalse($this->should_fail);
        $this->assertTrue($this->should_success);
    }

    /**
     * @group testRejectException
     * @expectedException \RuntimeException
     * @expectedExceptionMessage Test exception in reject()
     * @memcheck
     */
    public function testRejectException() {
        $defer = new Deferred(function ($reason) {
            throw new \RuntimeException("Test exception in reject()");
        });
        $defer->then(function ($data, $error) {
            $this->fail("deferred already rejected");
        });
        $defer->cancel("just reject");
        $this->fail("should be exception");
    }

    /**
     * @group testError
     * @memcheck
     */
    public function testError() {
        $defer = new Deferred(function () {
            $this->fail("deferred already done");
        });
        $defer->then(function ($data, $error) {
            $this->should_success = true;
            $this->assertNull($data);
            $this->assertException($error, "some error", 2);
        });
        $defer->fail(new \Exception("some error", 2));
        $this->assertTrue($this->should_success);
    }

    /**
     * @group testErrorException
     * @expectedException \RuntimeException
     * @expectedExceptionMessage Test exception in then()
     * @memcheck
     */
    public function testErrorException() {
        $defer = new Deferred(function () {
            $this->fail("deferred already done");
        });
        $defer->then(function ($data, $error) {
            throw new \RuntimeException("Test exception in then()");
        });
        $defer->fail(new \Exception("some error", 2));
        $this->fail("should be exception");
    }

    /**
     * @group testRejectAfterResolve
     * @expectedException \LogicException
     * @expectedExceptionMessage Failed to cancel finished deferred object
     * @memcheck
     */
    public function testRejectAfterResolve() {
        $defer = new Deferred(function () {
            $this->fail("deferred already done");
        });
        $defer->then(function ($data, $error) {
            // throw new \RuntimeException("Test exception in then()");
        });
        $defer->done("some result");
        $defer->cancel("bad reason");
        $this->fail("should be exception");
    }

    /**
     *
     * @memcheck
     */
    public function _testThenAfterDone() {
        $defer = new Deferred(function () {
        });
        $defer->done("already done");
        $defer->then(function ($data, $error) {
            $this->assertSame("already done", $data);
            $this->assertNull($error);
        });
    }
}