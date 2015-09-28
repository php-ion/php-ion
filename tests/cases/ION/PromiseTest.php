<?php

namespace cases\ION;

use ION\Promise;
use ION\Test\Callback;
use ION\Test\TestCase;

class PromiseTest extends TestCase {

    public static function staticMethod() {

    }

    public function method() {

    }

    /**
     *
     * @memcheck
     */
    public function testCreate() {
        $promise1 = new Promise(null, __CLASS__."::staticMethod", [$this, 'method']);
        $promise2 = new Promise('intval', null, [__CLASS__, 'staticMethod']);
        $promise3 = new Promise(function() {}, new Callback(function() {}, false), null);
    }

    /**
     *
     * @memcheck
     */
    public function testThen() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise->then(function() {}, function() {}, function() {});
    }
    /**
     * @group dev
     * @memcheck
     */
    public function testManyThens() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise
            ->then(function() {}, null, null)
            ->then(null, function() {}, null)
            ->then(null, null, function() {})
            ->then(function() {}, null, function() {})
            ->then(null, function() {}, null)
            ->then(function() {}, function() {}, null)
            ->then(null, function() {}, function() {})
            ;
    }
}