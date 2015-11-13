<?php

namespace ION;

use ION\Test\TestCase;

class SequenceTest extends TestCase {

    /**
     * @memcheck
     */
    public function testCreate() {
        new Sequence();
        new Sequence(function () {});
    }

    /**
     * @memcheck
     */
    public function testRushInvoke() {
        $seq = new Sequence(function ($x) {
            return $x + 10;
        });
        $seq
            ->then(function ($x) {
                return $x + 100;
            })
            ->onDone(function ($x) {
                $this->data["x1"] = $x;
            });

        $seq
            ->then(function ($x) {
                return $x + 1000;
            })
            ->onDone(function ($x) {
                $this->data["x2"] = $x;
            });

        $seq(1);

        $this->assertEquals([
            "x1" =>  111,
            "x2" => 1011
        ], $this->data);

        $seq(2);

        $this->assertEquals([
            "x1" =>  112,
            "x2" => 1012
        ], $this->data);
    }

	/**
	 * @group dev
	 * @memc heck
	 */
	public function testSequenceAsPromise() {
		$seq = new Sequence(function ($x) {
//			$this->out("work seq");
			return $x + 10;
		});

		$this->promise(function () use ($seq) {
//			$this->out("await seq");
			$this->data["seq.result.1"] = yield $seq;
//			$this->out("done seq");
		}, false, 2);

//		$this->promise(function () use ($seq) {
//			$this->out("run seq");
			$seq(1);
//		}, false, 3);
	}
}