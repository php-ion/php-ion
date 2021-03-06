<?php

namespace ION\Process;


use ION\Process;
use ION\Process\IPC\Message;
use ION\Test\TestCase;


class ChildProcessTest extends TestCase {

    /**
     * @memcheck
     */
    public function testInit() {
        $worker = new ChildProcess();
        $this->assertEquals(0, $worker->getPID());
        $this->assertFalse($worker->isAlive());
        $this->assertFalse(Process::hasParentIPC());
        $this->assertNull(Process::getParentIPC());
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

            $this->data["exit"]["has"]       = Process::hasChildProcess($w->getPID());
            $this->data["exit"]["get"]       = Process::getChildProcess($w->getPID());
            $this->data["exit"]["childs"]    = Process::getChildProcesses();

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

            $this->data["started"]["has"]       = Process::hasChildProcess($w->getPID());
            $this->data["started"]["get"]       = Process::getChildProcess($w->getPID());
            $this->data["started"]["childs"]    = Process::getChildProcesses();
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

        $this->data["begin"]["has"]       = Process::hasChildProcess($worker->getPID());
        $this->data["begin"]["get"]       = Process::getChildProcess($worker->getPID());
        $this->data["begin"]["childs"]    = Process::getChildProcesses();

        $this->loop();

        usleep(10000); // ru: ждем что бы дать время буферу вывода дочернего процесса сплюнуть все в stdout/stderr

//        $this->out($this->data);

        $this->assertEquals([
            "pid"       => 0,
            "status"    => -1,
            "alive"     => false,
            "finished"  => false,
            "signaled"  => false,
            "started"   => false,
            "connected" => true,
            "has"       => false,
            "get"       => NULL,
            "childs"    => []
        ], $this->data["begin"], "Begin");

        $this->assertEquals([
            "pid"       => $worker->getPID(),
            "status"    => -1,
            "alive"     => true,
            "finished"  => false,
            "signaled"  => false,
            "started"   => true,
            "connected" => true,
            "has"       => true,
            "get"       => $worker,
            "childs"    => [$worker->getPID() => $worker]
        ], $this->data["started"], "Started");

        $this->assertEquals([
            "pid"       => $worker->getPID(),
            "status"    => 0,
            "alive"     => false,
            "finished"  => true,
            "signaled"  => false,
            "started"   => true,
            "connected" => false,
            "has"       => false,
            "get"       => NULL,
            "childs"    => []
        ], $this->data["exit"], "Exit");
    }

    public function testCommunication() {
        $worker = new ChildProcess();
        $worker->getIPC()->whenIncoming()->then(function (Message $message) {

        });
    }
}