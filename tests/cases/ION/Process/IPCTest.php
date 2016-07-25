<?php

namespace ION\Process;


use ION\Test\TestCase;

class IPCTest extends TestCase {

    /**
     * @memcheck
     * @group dev
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

//    public function
}