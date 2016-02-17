<?php

namespace ION\Process;


use ION\Test\TestCase;

class WorkerTest extends TestCase {

    /**
     * @memcheck
     */
    public function testCreate() {
        $worker = new Worker();
        $this->assertEquals(0, $worker->getPID());
        $this->assertFalse($worker->isAlive());
        $this->assertFalse($worker->isStarted());
        $this->assertFalse($worker->isSignaled());
        $this->assertEquals(-1, $worker->getExitStatus());
        $worker->onExit()->then(function (Worker $w) {
            // do something
        });
        $worker->onMessage()->then(function ($msg) {
            // do something
        });
        $worker->run(function (Worker $w) {
            // do something
        });
    }

    /**
     * @group dev
     */
    public function _testSpawn() {
        $worker = new Worker();
        $worker->run(function () {
            usleep(100000);
            exit(12);
        });
        $worker->onExit()->then(function (Worker $w) {
            $this->data["is_exit"]  = $w->isAlive();
            $this->data["is_child"] = $w->isChild();
            $this->data["status"]   = $w->getExitStatus();
            $this->stop();
        });

        $this->loop();
    }
}