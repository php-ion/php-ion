<?php

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

class Worker {
    public function getStartedTime() : float {}

    public function getPID() : int {}

    public function isStarted() : bool {}

    public function isAlive() : bool {}

    public function isSignaled() : bool {}

    public function getSignal() : int {}

    public function getExitStatus() : int {}

    public function isMaster() : bool {}

    public function isChild() : bool {}

//    public function setCWD(string $path) : self {}
//
//    public function getCWD() : string {}
//
//    public function setPriority(int $prio) : self {}
//
//    public function getPriority() : int {}
//
//    public function setUser(string $user_name, string $group_name) : self {}
//    public function getUserName() : string {}
//
//    public function getGroupName() : string {}

    public function run(callable $cb) : self {}
    public function onConnect() : Promise {}
    public function onDisconnect() : Promise {}

    public function onExit() : Sequence {}

    public function onMessage() : Sequence {}

    public function message(string $name) {}
}