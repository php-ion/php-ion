<?php

namespace ION\Process;


use ION\Process\IPC\Message;
use ION\Test\TestCase;

class IPCTest extends TestCase {

    /**
     * @var IPC
     */
    public $tmp;

    /**
     * @group dev
     * @memcheck
     */
    public function testCreate() {

        list($one, $two) = IPC::create(strtolower("ONE1"), strtolower("TWO2"));
        /* @var IPC $one */
        /* @var IPC $two */
        $this->assertInstanceOf(IPC::class, $one);
        $this->assertInstanceOf(IPC::class, $two);

        $one->whenDisconnected()->then(function () {});
        $one->whenIncoming()->then(function () {});

        $this->assertEquals("one1", $one->getContext());
        $this->assertEquals("two2", $two->getContext());

    }

    public function message($data, $ctx) {
        $msg = new Message();
        $msg->context = $ctx;
        $msg->data = $data;
        return $msg;
    }

    /**
     * @memcheck
     */
    public function testCommunicate() {
        list($one, $two) = IPC::create((string)"one1", (string)"two2");
        /* @var IPC $one */
        /* @var IPC $two */
        $one->whenIncoming()->then(function (Message $data) {
            $this->data[] = $data;
        });
        \ION::await(0.02)->then(function () use($two) {
            $two->send("test1");
        });
        \ION::await(0.04)->then(function () use($two) {
            $two->send("test2");
        });
        \ION::await(0.06)->then(function () use($two) {
            $this->stop();
        });

        \ION::dispatch();
        $this->assertEquals([
            $this->message("test1", "one1"),
            $this->message("test2", "one1"),
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testDisconnect() {
        list($one, $this->tmp) = IPC::create((string)"one1", (string)"two2");
        /* @var IPC $one */
        /* @var IPC $two */
        $one->whenIncoming()->then(function ($data) {
            $this->data[] = $data;
        });
        $one->whenDisconnected()->then(function ($ctx) {
            $this->data[] = $ctx;
        });
        \ION::await(0.02)->then(function () {
            $this->tmp->send("test1");
        });
        \ION::await(0.03)->then(function () {
            unset($this->tmp);
        });
        \ION::await(0.04)->then(function () {
            $this->stop();
        });
        \ION::dispatch();
        $this->assertEquals([
            $this->message("test1", "one1"),
            "one1"
        ], $this->data);
    }
}