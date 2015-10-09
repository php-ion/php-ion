<?php

namespace cases\ION\Promise;


use ION\Promise\Result;
use ION\Test\TestCase;

class ResultTest extends TestCase {


    /**
     * @memcheck
     */
    public function testStore() {
        $result = new Result(1);
    }

    /**
     * @memcheck
     */
    public function testGetData() {
        $result = new Result(1);
        $this->assertEquals(1, $result->getData());
    }
}