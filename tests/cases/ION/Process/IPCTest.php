<?php

namespace ION\Process;


use ION\Test\TestCase;

class IPCTest extends TestCase {

    /**
     * @memcheck
     */
    public function testCreate() {

        list($one, $two) = IPC::create("one1", "two2");
        /* @var IPC $one */
        /* @var IPC $two */
        $this->assertInstanceOf(IPC::class, $one);
        $this->assertInstanceOf(IPC::class, $two);

        $one->connected()->then(function () {});
        $one->disconnected()->then(function () {});
        $one->message()->then(function () {});

        $this->assertEquals("one1", $one->getContext());
        $this->assertEquals("two2", $two->getContext());

    }

    /**
     * @group dev
     * @memcheck
     */
    public function testCommunicate() {
        list($one, $two) = IPC::create("one1", "two2");
        /* @var IPC $one */
        /* @var IPC $two */
        $one->message()->then(function ($data) {
            $this->data[] = $data;
        });
        \ION::await(0.05)->then(function () use($two) {
            $two->send("test1");
        });
        \ION::await(0.1)->then(function () use($two) {
            $two->send("test2");
        });
        \ION::await(0.15)->then(function () use($two) {
            $this->stop();
        });

        \ION::dispatch();
        $this->assertEquals([
            "test1", "test2"
        ], $this->data);
    }
}