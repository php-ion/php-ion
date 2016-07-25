<?php

namespace ION\Process;


use ION\Promise;

class ChildProcess  {
    public function getStartedTime() : float {}

    public function getPID() : int {}

    public function isSignaled() : bool {}

    public function getSignal() : int {}

    public function getExitStatus() : int {}

    public function released() : Promise {}

    public function run(callable $callback) : self {}

    public function ipc() : IPC {}

}