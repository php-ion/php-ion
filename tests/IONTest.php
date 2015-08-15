<?php


class IONTest extends ION\TestCase {

    /**
     * @memcheck
     */
    public function testAwait() {
        $begin = microtime(1);
        ION::await(0.5)->then(function() {
//            var_dump("do something");
            ION::stop();
        });
//        ION::stop(10);
        ION::dispatch();
//        var_dump("Done: ".(microtime(1) - $begin));
    }
}