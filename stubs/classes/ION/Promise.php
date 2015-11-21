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
    public static function invoke(callable $callable, ...$args) {}

    /**
     * Promise constructor.
     * @param callable $done
     * @param callable $fail
     */
    public function __construct(callable $done = null, callable $fail = null) {}

    /**
     * then(callable $done = null, callable $fail = null)
     * then(Promise $handler)
     *
     * @param Promise|callable $done
     * @param callable $fail
     * @return Promise
     */
    public function then($done = null, callable $fail = null) : Promise {}

    /**
     * @param Promise|string $handler
     * @return self
     */
    public function forget($handler) : self {}

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
     * @return string one of pending, done, canceled, failed, processing
     */
    public function getState() : string {}

    /**
     *
     */
    public function getFlags() : int {}

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