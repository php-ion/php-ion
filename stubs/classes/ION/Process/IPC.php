<?php

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

class IPC {

    /**
     * @param mixed $ctx1
     * @param mixed $ctx2
     *
     * @return array|IPC[]
     */
    public static function create($ctx1 = null, $ctx2 = null) : array {}

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

    /**
     * @return mixed
     */
    public function getContext() {}

    public function send($data) {}
}