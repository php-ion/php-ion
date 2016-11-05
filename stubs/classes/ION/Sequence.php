<?php

namespace ION;

class Sequence extends Promise {

    /**
     * Set initial callback: function (...$arg) {}
     *
     * @param callable $starter
     * @param callable $release
     */
    public function __construct(callable $starter = null, callable $release = null) { }

    /**
     * @param mixed ...
     */
    public function __invoke(...$args) { }
}