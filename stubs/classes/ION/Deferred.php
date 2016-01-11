<?php

namespace ION;

/**
 * Ticket for deferred actions
 */
class Deferred extends ResolvablePromise {

    /**
     * @param callable $canceler
     */
    public function __construct(callable $canceler = null) { }


    /**
     * Cancel deferred object
     *
     * @param string $reason
     */
    public function cancel($reason) : self { }


    /**
     * @param float $sec
     *
     * @return Promise
     */
    public function timeout(float $sec) : self { }


    public function __clone() { }
}