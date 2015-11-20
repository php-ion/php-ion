<?php

namespace ION;


/**
 * @since 1.0
 * @package ION
 */
class Promise {

    public static function all(...$values) {}

    /**
     * @param ...$values
     */
    public static function any(...$values) {}

    /**
     * @param callable $callable
     * @param ...$args
     */
    public static function call(callable $callable, ...$args) {}

//    public function __construct(callable $done = null, callable $fail = null, callable $progress = null) {}
    public function __construct(callable ...$callbacks) {}

    /**
     * then(callable $done = null, callable $fail = null, callable $progress = null)
     * then(Promise $handler)
     *
     * @param callable $done
     * @param callable $fail
     * @param callable $progress
     * @return Promise
     */
    public function then(...$handlers) : Promise {}

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
     * @return string one of pending, done, canceled, failed, processing
     */
    public function getState() : string {}

    /**
     *
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

    /**
     * @return mixed
     */
    public function getResult() {}

    /**
     * @param string $name
     * @return Promise
     */
    public function setName(string $name) : self {}

}