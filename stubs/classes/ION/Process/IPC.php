<?php

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

class IPC {

    /**
     * @param mixed $ctx1 context for IPC channel one
     * @param mixed $ctx2 context for IPC channel two
     *
     * @return array|IPC[]
     */
    public static function create($ctx1 = null, $ctx2 = null) : array {}

    /**
     * @return bool
     */
    public function isConnected() : bool {}

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

    /**
     * @return mixed
     */
    public function getContext() {}

    public function send($data) {}
}