<?php

namespace ION;


use ION\Sequence\Quit;

class Sequence extends Promise {

    public static function quit() : Quit {}

    /**
     * Set initial callback: function (...$arg) {}
     *
     * @param callable $initial
     */
    public function __construct(callable $initial = null) { }

    /**
     * @param mixed ...
     */
    public function __invoke(...$args) { }
}