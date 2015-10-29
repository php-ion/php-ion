<?php

namespace ION;


class Sequence extends Promise {

    /**
     * Set initial callback: function ($arg) {}
     * @param callable $initial
     */
    public function __construct(callable $initial) {}

    /**
     * @param mixed ...
     */
    public function __invoke(...$arg) {}
}