<?php

namespace ION;


class PromiseMap extends Promise {

    /**
     * Set initial callback: function ($arg) {}
     * @param callable $initial
     */
    public function __construct(callable $initial) {}

    /**
     * @param int $arg
     */
    public function __invoke($arg) : void {}
}