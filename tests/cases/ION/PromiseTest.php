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
        $promise  = new Promise();
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

    /**
     * @memcheck
     */
    public function testManyChilds() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise->then(function() {}, null, null);
        $promise->then(null, function() {}, null);
        $promise->then(null, null, function() {});
        $promise->then(function() {}, null, function() {});
        $promise->then(null, function() {}, null);
        $promise->then(function() {}, function() {}, null);
        $promise->then(null, function() {}, function() {});
    }


    /**
     * @memcheck
     */
    public function testDone() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise->onDone(function() {});
        $promise
            ->onDone(function() {})
            ->onDone(function() {});
    }


    /**
     * @memcheck
     */
    public function testFail() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise->onFail(function() {});
        $promise
            ->onFail(function() {})
            ->onFail(function() {});
    }

    /**
     * @memcheck
     */
    public function testProgress() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise->onProgress(function() {});
        $promise
            ->onProgress(function() {})
            ->onProgress(function() {});
    }

    /**
     * @memcheck
     */
    public function testMixedChain() {
        $promise = new Promise(function() {}, function() {}, function() {});
        $promise->then(function() {}, function() {}, function() {});
        $promise->onDone(function() {})
            ->then(function() {})
            ->onFail(function() {});
        $promise->onFail(function() {})
            ->onDone(function() {})
            ->then(function() {}, function() {});
        $promise->onProgress(function() {});
    }

    /**
     * @group dev
     * @memcheck
     */
    public function testSimpleDone() {
        $promise = new Promise(function($x) {
            $this->data["x0"] = $x;
            return $x + 1;
        }, function($error) {
            $this->data["x0.error"] = $error;
        }, function($info) {
            $this->data["x0.progress"] = $info;
        });

        $promise
            ->then(function (\StdClass $x) {
                $this->data["x1"] = $x;
                return $x + 10;
            })
            ->then(function ($x2, \Exception $error = null) {
                $this->data["x2"] = $x2;
                $this->data["x2.error"] = $error;
                return $x2 + 100;
            })
            ->onDone(function ($x) {
                $this->data["result"] = $x;
            })
            ->onFail(function ($error) {
                $this->data["error"] =$error;
            })
        ;

        $promise->done(1);
        $this->assertSame([
            'x0' => 1,
            'x2' => 2,
            'x2.error' => null,
            'result' => 102
        ], $this->data);
    }


}