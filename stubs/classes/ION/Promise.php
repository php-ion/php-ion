<?php

namespace ION;


class Promise {

    public function __construct(callable $done = null, callable $fail = null, callable $progress = null) {}

    /**
     * @param callable $done
     * @param callable $fail
     * @param callable $progress
     * @return self
     */
    public function then(callable $done = null, callable $fail = null, callable $progress = null) {}

    /**
     * @param callable $callback
     * @return self
     */
    public function done(callable $callback) {}

    /**
     * @param callable $callback
     * @return self
     */
    public function fail(callable $callback) {}

    /**
     * @param callable $callback
     * @return self
     */
    public function progress(callable $callback) {}

    /**
     * @return self
     */
    public function cancel() {}

    /**
     * @param int|float $sec
     * @return self
     */
    public function timeout($sec) {}

    /**
     * @param Promise $head
     * @return array
     */
    public static function analyseChain(self $head) {}
}