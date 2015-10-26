<?php

namespace ION;
use ION\Promise\Result;

declare(strict_types=1);

class Promise {

//    public function __construct(callable $done, callable $progress = null) {}
//    public function __construct(callable $done = null, callable $fail = null, callable $progress = null) {}
    public function __construct(callable ...$callbacks) {}

    /**
     * @param callable $done
     * @param callable $fail
     * @param callable $progress
     * @return Promise
     */
    public function then(callable ...$callbacks) : Promise {}

    /**
     * @param callable $callback
     * @return Promise
     */
    public function onDone(callable $callback) : Promise {}

    /**
     * @param callable $callback
     * @return Promise
     */
    public function onFail(callable $callback) : Promise {}

    /**
     * @param callable $callback
     * @return Promise
     */
    public function onProgress(callable $callback) : Promise {}

    /**
     * @todo
     */
    public function getState() : string {}

    /**
     * @todo
     */
    public function getFlags() : int {}

    /**
     * @todo
     * @param mixed $info
     * @return Promise
     */
    public function notify(mixed $info) : Promise {}

    /**
     * @param float $sec
     * @return Promise
     */
    public function timeout(float $sec) : Promise {}

}