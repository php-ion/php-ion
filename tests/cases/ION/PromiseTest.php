<?php
//declare(strict_types=1);

namespace ION;

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
        $promise1 = new Promise(null, __CLASS__."::staticMethod");
        $promise2 = new Promise('intval', null);
        $promise3 = new Promise(function() {}, new Callback(function() {}, false));
        $promise3->setName("name");
    }

    /**
     * @memcheck
     */
    public function testThen() {
        $promise = new Promise(function() {}, function() {});
        $promise->then(function() {}, function() {});
    }

    /**
     *
     * @memcheck
     */
    public function testThenThenThen() {
        $promise = new Promise(function() {}, function() {});
        $promise
            ->then(function() {}, null)
            ->then(null, function() {})
            ->then(null, null)
            ->then(function() {}, null)
            ->then(null, function() {})
            ->then(function() {}, function() {})
            ->then(null, function() {})
            ;
    }

    /**
     *
     * @memcheck
     */
    public function testParallelThenThenThen() {
        $promise = new Promise(function() {}, function() {});
        $promise->then(function() {}, null);
        $promise->then(null, function() {});
        $promise->then(null, null);
        $promise->then(function() {}, null);
        $promise->then(null, function() {});
        $promise->then(function() {}, function() {});
        $promise->then(null, function() {});
    }


    /**
     * @memcheck
     */
    public function testDone() {
        $promise = new Promise(function() {}, function() {});
        $promise->onDone(function() {});
        $promise
            ->onDone(function() {})
            ->onDone(function() {});
    }


    /**
     * @memcheck
     */
    public function testFail() {
        $promise = new Promise(function() {}, function() {});
        $promise->onFail(function() {});
        $promise
            ->onFail(function() {})
            ->onFail(function() {});
    }

    /**
     * @memcheck
     */
    public function testProperties() {
        $promise = new Promise();
        $promise->a = 1;
        $promise->b = 2;
        $this->assertEquals(1, $promise->a);
        $this->assertEquals(2, $promise->b);
    }

    /**
     * @memcheck
     */
    public function testCloneable() {
        $promise = new ResolvablePromise(function() {}, function() {});
        $promise->a = 1;
        $promise->then(function() {})->then(function () {}, function () {});
        $promise->then(function() {});

        $clone = clone $promise;
        $this->assertEquals($promise->a, $clone->a);
        $clone->done(1);
        $promise->done(1);

    }

    /**
     * @memcheck
     */
    public function testMixedChain() {
        $promise = new Promise(function() {}, function() {});
        $promise->then(function() {}, function() {});
        $promise->onDone(function() {})
            ->then(function() {})
            ->onFail(function() {});
        $promise->onFail(function() {})
            ->onDone(function() {})
            ->then(function() {}, function() {});
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
        });

        $promise
            ->then(function (\StdClass $x) {
                $this->data["x1"] = $x;
                return $x + 10;
            })
            ->then(function ($x2) {
                $this->data["x2"] = $x2;
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
            'result' => 102
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testWhenResolved() {
        $promise  = new ResolvablePromise();
        $promise->done("already done");
        $promise->then(function ($result) {
            $this->data["result"] = $this->describe($result);
        });
        $this->assertEquals([
            'result' => "already done"
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testDeferredThenDeferred() {
        $d1 = new Deferred(function() {
            $this->data["d1.cancel"] = true;
        });
        $d2 = new Deferred(function() {
            $this->data["d2.cancel"] = true;
        });
        $d2->then(function ($result) {
            $this->data["result"] = $this->describe($result);
        }, function ($result) {
            $this->data["error"] = $this->describe($result);
        });

        $d1->then($d2);

        $d2->done("iddqd");

        $this->assertSame([
            'result' => "iddqd",
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
     * @group dev
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

    public static function simpleGenerator() {
        yield 1;
        yield 2;
        yield 3;
        return 4;
    }

	/**
	 * @param callable $action
	 * @param bool $stop
	 * @return \ION\ResolvablePromise
	 */
    public function mkpromise(callable $action) {
        $promise = new ResolvablePromise();
        $promise
            ->then($action)
            ->onDone(function ($result) {
                $this->data["result"] = $this->describe($result);
            })
            ->onFail(function ($error) {
                $this->data["error"] = $this->describe($error);
            });

        return $promise;
    }

    public function providerPromiseActions() {
        return array(
            /* Generator vs generator */
            [
                function () {
                    return self::simpleGenerator();
                },
                [
                    'result' => [
                        'object' => 'Generator'
                    ]
                ]
            ], [
                function () {
                    $x = yield from self::simpleGenerator();
                    return $x;
                },
                [
                    'result' => 4
                ]
            ], [
                function () {
                    return yield from self::simpleGenerator();
                },
                [
                    'result' => 4
                ]
            ], [
                function () {
                    $x = yield self::simpleGenerator();
                    return $x;
                },
                [
                    'result' => [
                        'object' => 'Generator'
                    ]
                ]
            ], [
                function () {
                    return yield self::simpleGenerator();
                },
                [
                    'result' => [
                        'object' => 'Generator'
                    ]
                ]
            ],
        );
    }

    /**
     * @memcheck
     * @dataProvider providerPromiseActions
     * @param callable $action
     * @param array $result
     * @param mixed $arg
     */
    public function testPromiseAction(callable $action, array $result, $arg = null) {
        $this->mkpromise($action)->done($arg);
        $this->assertEquals($result, $this->data);
    }


    public function providerTypeHintInAction($intern = false) {
        $std = new \StdClass;
        if($intern) {
            $class = Debug::class;
            $no_hint                  = "$class::noHint";
            $SplDoublyLinkedList_hint = "$class::SplDoublyLinkedListHint";
            $ArrayAccess_hint         = "$class::ArrayAccessHint";
            $SplQueue_hint            = "$class::SplQueueHint";
            $array_hint               = "$class::arrayHint";
            $ArrayObject_hint         = "$class::ArrayObjectHint";
            $callable_hint            = "$class::callableHint";
            $int_hint                 = "$class::intHint";
            $double_hint              = "$class::doubleHint";
            $string_hint              = "$class::stringHint";
            $bool_hint                = "$class::boolHint";
        } else {
            $no_hint                  = function ($elem) { return true; };
            $SplDoublyLinkedList_hint = function (\SplDoublyLinkedList $elem) { return true; };
            $ArrayAccess_hint         = function (\ArrayAccess $elem) { return true; };
            $SplQueue_hint            = function (\SplQueue $elem) { return true; };
            $array_hint               = function (array $elem) { return true; };
            $ArrayObject_hint         = function (\ArrayObject $elem) { return true; };
            $callable_hint            = function (callable $elem) { return true; };
            $int_hint                 = function (int $elem) { return true; };
            $double_hint               = function (float $elem) { return true; };
            $string_hint              = function (string $elem) { return true; };
            $bool_hint                = function (bool $elem) { return true; };
        }
        return array(
            // objects
            // true
            [true, new \SplDoublyLinkedList(), $no_hint ],
            [true, new \SplDoublyLinkedList(), $SplDoublyLinkedList_hint  ],
            [true, new \SplQueue(), $SplDoublyLinkedList_hint  ],
            [true, new \SplQueue(), $ArrayAccess_hint ],
            // false
            [false, new \SplDoublyLinkedList(), $SplQueue_hint ],
            [false, new \SplPriorityQueue(), $SplQueue_hint ],
            [false, new \ArrayObject(), $SplQueue_hint ],

            // arrays
            // true
            [true, [], $no_hint  ],
            [true, [], $array_hint ],
            // false
            [false, [], $ArrayObject_hint  ],
            [false, new \ArrayObject(), $array_hint  ],

            // callables
            // true
            [true, function () {}, $no_hint ],
            [true, function () {}, $callable_hint ],
            [true, __CLASS__ . "::simpleGenerator", $callable_hint  ],
            [true, [__CLASS__, "simpleGenerator"], $callable_hint ],
            [true, [$this, __METHOD__], $callable_hint  ],
            [true, "intval", $callable_hint  ],
            // false
            [false, "null", $callable_hint  ],
            [false, ["zz", "zz"], $callable_hint  ],
            [false, "zz:zz", $callable_hint ],
            [false, [$this, "zz"], $callable_hint  ],

            // integers
            // true
            [true, 5, $no_hint  ],
            [true, 5, $int_hint ],
            [true, "5", $int_hint  ],
            [true, "5.5", $int_hint  ],
            [true, 5.0, $int_hint  ],
            [true, 5.5, $int_hint  ],
            [true, "5.z", $int_hint  ],
            [true, true, $int_hint  ],
            [true, false, $int_hint ],
            // false
            [false, null,  $int_hint ],
            [false, "z",   $int_hint  ],
            [false, [],    $int_hint  ],
            [false, $std,  $int_hint  ],
            [false, STDIN, $int_hint  ],

            // floats
            // true
            [true, 5.5,   $no_hint ],
            [true, 5.5,   $double_hint ],
            [true, 5,     $double_hint  ],
            [true, 5.0,   $double_hint  ],
            [true, "5",   $double_hint  ],
            [true, "5.5", $double_hint  ],
            [true, true,  $double_hint  ],
            [true, false, $double_hint  ],
            // false
            [false, null,  $double_hint  ],
            [false, "5.z", $double_hint  ],
            [false, "z.z", $double_hint  ],
            [false, [],    $double_hint  ],
            [false, $std,  $double_hint  ],
            [false, STDIN, $double_hint  ],

            // strings
            // true
            [true, "str",   $no_hint ],
            [true, "str",   $string_hint ],
            [true, 5,       $string_hint  ],
            [true, 5.5,     $string_hint  ],
            [true, true,    $string_hint  ],
            [true, false,   $string_hint  ],
            [true, new \Exception(),   $string_hint  ],
            // false
            [false, new \SplDoublyLinkedList(),   $string_hint  ],
            [false, STDIN,   $string_hint  ],
            [false, [],      $string_hint  ],

            // booleans
            [true, "str",   $no_hint ],
            [true, "str",   $bool_hint ],
            [true, 5,       $bool_hint  ],
            [true, 5.5,     $bool_hint  ],
            [true, true,    $bool_hint  ],
            [true, false,   $bool_hint  ],
            // false
            [false, new \SplDoublyLinkedList(), $bool_hint  ],
            [false, STDIN,  $bool_hint  ],
            [false, [],     $bool_hint  ],

        );
    }

    /**
     * @memcheck
     * @dataProvider providerTypeHintInAction
     * @param bool $ok
     * @param callable $action
     * @param mixed $arg
     */
    public function testTypeHintInAction($ok, $arg, callable $action) {
        @$this->mkpromise($action)->done($arg); // notices generate memory-leak in the phpunit
        if($ok) {
            $this->assertEquals([
                "result" => true
            ], $this->data);
        } else {
            $this->assertEquals([
                "result" => $this->describe($arg)
            ], $this->data);
        }
    }

    public function providerTypeHintInActionIntern() {
        return $this->providerTypeHintInAction(true);
    }

    /**
     * @memcheck
     * @dataProvider providerTypeHintInActionIntern
     */
    public function testTypeHintInActionIntern($ok, $arg, callable $action) {
        $this->testTypeHintInAction($ok, $arg, $action);
    }

	/**
	 *
	 * @memcheck
	 */
	public function testPromiseResolved() {
		$prom = new ResolvablePromise();
		$this->promise(function () use ($prom) {
			$prom->done(1);
			$this->data["promise"] = yield $prom;
			return $prom;
		}, false);

		$this->assertSame([
			"promise" => 1,
			"result" => 1
		], $this->data);
	}

	/**
	 * @memcheck
	 */
	public function testPromiseResolvedClone() {
		$prom = new ResolvablePromise();
		$this->promise(function () use ($prom) {
			$prom->done(1);
			$p = clone $prom;
			$this->data["promise"] = yield $p;
			return $prom;
		}, false);

		$this->assertSame([
			"promise" => 1,
			"result" => 1
		], $this->data);
	}

	/**
	 * @memcheck
	 */
	public function testPromiseYield() {
		$prom = new ResolvablePromise(function ($x) {
			return $x + 10;
		});
		$this->promise(function () use ($prom) {
			$this->data["promise"] = yield $prom;
		}, false);

		$prom->done(1);

		$this->assertSame([
			"promise" => 11
		], $this->data);
	}

	/**
	 * @memcheck
	 */
	public function testPromiseYieldClone() {
		$prom = new ResolvablePromise(function ($x) {
			return $x + 10;
		});
		$this->promise(function () use ($prom) {
			$this->data["promise"] = yield $prom;
		}, false);

		$p = clone $prom;
		$p->done(2);
		$prom->done(1);

		$this->assertSame([
			"promise" => 11
		], $this->data);
	}

    /**
     * @memcheck
     */
    public function testForget() {
        $promise1 = new ResolvablePromise(function () {
            $this->data["promise.1"] = true;
        });
        $promise2 = new Promise(function () {
            $this->data["promise.2"] = true;
        });
        $promise3 = new Promise(function () {
            $this->data["promise.3"] = true;
        });
        $promise1->then($promise2);
        $promise1->forget($promise3);
        $promise1->forget($promise2);
        $promise1->done(null);

        $this->assertSame([
            "promise.1" => true
        ], $this->data);
    }


    /**
     * @memcheck
     */
    public function testForgetNamed() {
        $promise1 = new ResolvablePromise(function () {
            $this->data["promise.1"] = true;
        });
        $promise2 = new Promise(function () {
            $this->data["promise.2"] = true;
        });
        $promise2->setName("promise2");
        $promise3 = new Promise(function () {
            $this->data["promise.3"] = true;
        });
        $promise3->setName("promise3");

        $promise1->then($promise2);
        $promise1->forget("promise3");
        $promise1->forget("promise2");
        $promise1->done(null);

        $this->assertSame([
            "promise.1" => true
        ], $this->data);
    }
}