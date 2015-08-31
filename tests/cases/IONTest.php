<?php

use ION\Test\TestCase;

class IONTest extends TestCase {

    /**
     * @memcheck
     */
    public function testAwait() {
        ION::await(0.5)->then(function() {
            ION::stop();
        });
        ION::dispatch();
    }
}