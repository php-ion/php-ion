<?php

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

class IPC {

    /**
     * @return IPC[]
     */
    public static function create() : array {}

    /**
     * @return bool
     */
    public function isConnected() : bool {}

    /**
     * @return bool
     */
    public function isDisconnected() : bool {}

    /**
     * @return Promise
     */
    public function connected() : Promise {}

    /**
     * @return Promise
     */
    public function disconnected() : Promise {}

    /**
     * @return float
     */
    public function getLastTime() : float  {}

    /**
     * @return Sequence
     */
    public function message() : Sequence {}

    public function send($data) {}
}