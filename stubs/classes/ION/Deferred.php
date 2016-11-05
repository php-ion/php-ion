<?php

namespace ION;

/**
 * A Deferred represents a computation or unit of work that may not have completed yet. Typically (but not always),
 * that computation will be something that executes asynchronously and completes at some point in the future.
 */
class Deferred extends ResolvablePromise {

    /**
     * @param callable $canceller
     * @param bool $protected
     */
    public function __construct(callable $canceller = null, bool $protected = true) { }


    /**
     * Cancel deferred object
     *
     * @param string $reason
     *
     * @return Deferred
     */
    public function cancel($reason) : self { }


    /**
     * @param float $sec
     *
     * @return Deferred
     */
    public function timeout(float $sec) : self { }


    public function __clone() { }
}