<?php

namespace ION\Process;


use ION\Test\TestCase;

class IPCTest extends TestCase {

    /**
     * @group dev
     */
    public function testCreate() {

        list($one, $two) = IPC::create();
        /* @var IPC $one */
        /* @var IPC $two */
//        $this->assertInstanceOf(IPC::class, $one);
//        $this->assertInstanceOf(IPC::class, $two);
//
//        $one->connected()->then(function () {});
//        $one->disconnected()->then(function () {});
//        $one->message()->then(function () {});

    }
}