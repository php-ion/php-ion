<?php

namespace ION\Process;


use ION\Process\IPC\Message;
use ION\Test\TestCase;


class ChildProcessTest extends TestCase {

    /**
     * @memc heck
     * @requires OS Darwin
     */
    public function testCreate() {
        $worker = new ChildProcess();
        $this->assertEquals(0, $worker->getPID());
        $this->assertFalse($worker->isAlive());
        $this->assertFalse($worker->isStarted());
        $this->assertFalse($worker->isSignaled());
        $this->assertEquals(-1, $worker->getExitStatus());

        $worker->whenExit()->then(function (ChildProcess $w) {
            // do something
        });
        $worker->whenStarted()->then(function (ChildProcess $w) {
            // do something
        });
        $worker->whenMessage()->then(function (Message $msg) {
            // do something
        });
        $worker->start(function (IPC $parent_ipc) {
            // do something
        });

        $this->loop();
    }

    /**
     */
    public function _testSpawn() {
        $this->data["master"] = getmypid();
        $worker = new Worker();
        $worker->onConnect()->then(function (Worker $w) {
            $this->data["connected"] = true;
        });
        $worker->onDisconnect()->then(function (Worker $w) {
            $this->data["disconnected"] = true;
        });
        $worker->run(function (Worker $w) {
            sleep(2);
            var_dump(getmypid().": i am exit");
            ob_flush();
            exit(12);
        });
        $worker->onExit()->then(function (Worker $w) {
            $this->data["cb_pid"] = getmypid();
            $this->data["child_pid"] = $w->getPID();
            $this->data["is_exit"]     = $w->isAlive();
            $this->data["is_child"]    = $w->isChild();
            $this->data["is_signaled"] = $w->isSignaled();
            $this->data["status"]      = $w->getExitStatus();
            $this->stop();
        });
        $this->loop();

        $this->assertEquals([
            'master' => getmypid(),
        ], $this->data);
    }

    public function _testMessaging() {
        $worker = new Worker();
        $worker->run(function () {
            usleep(100000);
            exit(0);
        });
        $worker->onExit()->then(function (Worker $w) {
            $this->data["status"]   = $w->getExitStatus();
            $this->stop();
        });

        $this->loop();
    }
}