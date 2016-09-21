<?php

namespace ION\Process;


use ION\Process\IPC\Message;
use ION\Test\TestCase;


class ChildProcessTest extends TestCase {

    /**
     * @group dev
     * @memcheck
     * @requires OS Darwin
     */
    public function testInit() {
        $worker = new ChildProcess();
        $this->assertEquals(0, $worker->getPID());
        $this->assertFalse($worker->isAlive());
        $this->assertFalse($worker->isStarted());
        $this->assertFalse($worker->isSignaled());
        $this->assertEquals(-1, $worker->getExitStatus());

        $worker->whenExit()->then(function (ChildProcess $w) {
            $this->data["exit"]["pid"]       = $w->getPID();
            $this->data["exit"]["status"]    = $w->getExitStatus();
            $this->data["exit"]["alive"]     = $w->isAlive();
            $this->data["exit"]["finished"]  = $w->isFinished();
            $this->data["exit"]["signaled"]  = $w->isSignaled();
            $this->data["exit"]["started"]   = $w->isStarted();
            $this->data["exit"]["connected"] = $w->getIPC()->isConnected();
            $this->stop();
            // do something else
        });
        $worker->whenStarted()->then(function (ChildProcess $w) {
            $this->data["started"]["pid"]       = $w->getPID();
            $this->data["started"]["status"]    = $w->getExitStatus();
            $this->data["started"]["alive"]     = $w->isAlive();
            $this->data["started"]["finished"]  = $w->isFinished();
            $this->data["started"]["signaled"]  = $w->isSignaled();
            $this->data["started"]["started"]   = $w->isStarted();
            $this->data["started"]["connected"] = $w->getIPC()->isConnected();
            // do something else

        });
        $this->assertInstanceOf(IPC::class, $worker->getIPC());
        $this->assertEquals($worker, $worker->getIPC()->getContext());

        $worker->start(function (IPC $ipc) {
            usleep(10000);
            // do something and exit
            exit;
        });

        $this->data["begin"]["pid"]       = $worker->getPID();
        $this->data["begin"]["status"]    = $worker->getExitStatus();
        $this->data["begin"]["alive"]     = $worker->isAlive();
        $this->data["begin"]["finished"]  = $worker->isFinished();
        $this->data["begin"]["signaled"]  = $worker->isSignaled();
        $this->data["begin"]["started"]   = $worker->isStarted();
        $this->data["begin"]["connected"] = $worker->getIPC()->isConnected();

        $this->loop();

        usleep(10000); // ru: ждем что бы дать время буферу вывода дочернего процесса сплюнуть все в stdout/stderr

        $this->assertEquals([
            "pid"       => 0,
            "status"    => -1,
            "alive"     => false,
            "finished"  => false,
            "signaled"  => false,
            "started"   => false,
            "connected" => true,
        ], $this->data["begin"]);

        $this->assertEquals([
            "pid"       => $worker->getPID(),
            "status"    => -1,
            "alive"     => true,
            "finished"  => false,
            "signaled"  => false,
            "started"   => true,
            "connected" => true,
        ], $this->data["started"]);

        $this->assertEquals([
            "pid"       => $worker->getPID(),
            "status"    => 0,
            "alive"     => false,
            "finished"  => true,
            "signaled"  => false,
            "started"   => true,
            "connected" => false,
        ], $this->data["exit"]);
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