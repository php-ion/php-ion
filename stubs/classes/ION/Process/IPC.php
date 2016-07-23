<?php

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

class IPC {

    /**
     * @param mixed $ctx
     *
     * @return array|IPC[]
     */
    public static function create($ctx) : array {}

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