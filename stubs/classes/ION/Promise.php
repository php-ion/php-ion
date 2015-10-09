<?php

namespace ION;

use ION\Promise\Result;

class Promise {

    /**
     * @param mixed $data
     * @return Result
     */
    public static function result($data) {}

//    public function __construct(callable $done, callable $progress = null) {}
//    public function __construct(callable $done = null, callable $fail = null, callable $progress = null) {}
    public function __construct(callable ...$callbacks) {}

    /**
     * @param callable $done
     * @param callable $fail
     * @param callable $progress
     * @return Promise
     */
    public function then(callable ...$callbacks) {}

    /**
     * @param callable $callback
     * @return Promise
     */
    public function onDone(callable $callback) {}

    /**
     * @param callable $callback
     * @return Promise
     */
    public function onFail(callable $callback) {}

    /**
     * @param callable $callback
     * @return Promise
     */
    public function onProgress(callable $callback) {}

    /**
     * @return Promise
     */
//    public function cancel() {}

    /**
     * @param int|float $sec
     * @return self
     */
//    public function timeout($sec) {}

}