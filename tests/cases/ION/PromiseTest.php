<?php

namespace cases\ION;

use ION;
use ION\Promise;
use ION\ResolvablePromise;
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
        $promise->then(function() {}, function() {}, function () {});
    }

    /**
     *
     * @memcheck
     */
    public function testThenThenThen() {
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
     *
     * @memcheck
     */
    public function testParallelThenThenThen() {
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
     * @memcheck
     */
    public function testSimpleChain() {
        $promise = new ResolvablePromise(function($x) {
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
                $this->data["error"] = $error;
            })
        ;

        $promise->done(1);
        $this->assertEquals([
            'x0' => 1,
            'x2' => 2,
            'x2.error' => null,
            'result' => 102
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testEmptyHeadChain() {
        $promise  = new ResolvablePromise();
        $promise->onDone(function ($result) {
            $this->data["result"] = $result;
        })->onFail(function ($error) {
            $this->data["error"] = $error;
        });
        $promise->done(2);
        $this->assertSame([
            'result' => 2,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testAwaitSuccessDeferred() {
        $promise = new ResolvablePromise(function($x) {
            $this->data["x0"] = $x;
            return $x + 1;
        });
        $promise
            ->then(function ($x) {
                $this->data["x1"] = $x;
                return ION::await(0.1);
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
                $this->stop();
            })
            ->onFail(function ($error) {
                $this->data["error"] = $error;
                $this->stop();
            })
        ;
        $promise->done(1);

        $this->loop();
        $this->assertEquals([
            'x0' => 1,
            'x1' => 2,
            'result' => true,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testAwaitSuccessPromise() {
        $promise = new ResolvablePromise(function($x) {
            $this->data["x0"] = $x;
            return $x + 1;
        });
        $promise
            ->then(function ($x) {
                $this->data["x1"] = $x;
                return \ION::await(0.1)->then(function($result) use ($x) {
                    $this->data["await"] = $result;
                    return $x + 10;
                });
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
                $this->stop();
            })
            ->onFail(function ($error) {
                $this->data["error"] = $error;
                $this->stop();
            })
        ;
        $promise->done(1);

        $this->loop(1);
        $this->assertEquals([
            'x0' => 1,
            'x1' => 2,
            'await' => true,
            'result' => 12,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testAwaitFailedPromise() {
        $promise = new ResolvablePromise();
        $promise
            ->then(function ($x) {
                $this->data["x1"] = $x;
                return \ION::await(0.1)->then(function($result) use ($x) {
                    $this->data["await"] = $result;
                    throw new \RuntimeException("problem description");
                });
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
                $this->stop();
            })
            ->onFail(function ($error) {
                /* @var \Exception $error */
                $this->data["error"] = $this->exception2array($error);
                $this->stop();
            })
        ;
        $promise->done(1);

        $this->loop();
        $this->assertEquals([
            'x1' => 1,
            'await' => true,
            'error' => [
                'exception' => 'RuntimeException',
                'message' => 'problem description',
                'code' => 0
            ],
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testYieldScalars() {
        $promise = new ResolvablePromise();
        $promise
            ->then(function ($x) {
                $this->data["x0"] = $x;
                $x = (yield $x + 1);
                $this->data["x1"] = $x;
                yield $x + 10;
                $this->data["x2"] = $x;
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
            })
            ->onFail(function ($error) {
                /* @var \Exception $error */
                $this->data["error"] = [
                    'class' => get_class($error),
                    'message' => $error->getMessage()
                ];
            })
        ;
        $promise->done(1);
        $this->assertEquals([
            "x0" => 1,
            "x1" => 2,
            "x2" => 2,
            "result" => null,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testYieldDeferred() {
        $promise = new ResolvablePromise();
        $promise
            ->then(function ($x) {
                $this->data["await"] = (yield ION::await(0.1));
                $this->data["x1"] = $x;
                $x = (yield $x + 10);
                $this->data["x2"] = $x;
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
                $this->stop();
            })
            ->onFail(function ($error) {
                /* @var \Exception $error */
                $this->data["error"] = $this->exception2array($error);
                $this->stop();
            })
        ;

        $promise->done(1);
        $this->loop();
        $this->assertEquals([
            "await" => true,
            "x1" => 1,
            "x2" => 11,
            "result" => null,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testYieldSuccessPromise() {
        $promise = new ResolvablePromise();
        $promise
            ->then(function ($x) {
                $this->data["x0"] = $x;
                $x = $x + (yield ION::await(0.1)->then(function ($result) {
                    $this->data["await"] = $result;
                    return 100;
                }));
                $this->data["x1"] = $x;
                $x = (yield $x + 10);
                $this->data["x2"] = $x;
                return $x;
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
                $this->stop();
            })
            ->onFail(function ($error) {
                /* @var \Exception $error */
                $this->data["error"] = $this->exception2array($error);
                $this->stop();
            })
        ;

        $promise->done(1);
        $this->loop();
        $this->assertEquals([
            "x0" => 1,
            "await" => true,
            "x1" => 101,
            "x2" => 111,
            "result" => 111,
        ], $this->data);
    }

    /**
     * @group dev
     * @memcheck
     */
    public function testYieldFailedPromise() {
        $promise = new ResolvablePromise();
        $promise
            ->then(function ($x) {
                try {
                    $this->data["x0"] = $x;
                    $x = $x + (yield ION::await(0.1)->then(function ($result) {
                        $this->data["await"] = $result;
                        throw new \RuntimeException("problem description");
                    }));
                    $this->data["x1"] = $x;
                } catch (\Exception $e) {
                    $this->data["failed"] = true;
                    throw $e;
                }
            })
            ->onDone(function ($result) {
                $this->data["result"] = $result;
                $this->stop();
            })
            ->onFail(function (\Throwable $error) {
                $this->data["error"] = $this->exception2array($error);
                $this->stop();
            })
        ;

        $promise->done(1);
        $this->loop();
        $this->assertEquals([
            "x0" => 1,
            "await" => true,
            "failed" => true,
            "error" => [
                'exception' => 'RuntimeException',
                'message' => "problem description",
                'code' => 0
            ]
        ], $this->data);
    }

}