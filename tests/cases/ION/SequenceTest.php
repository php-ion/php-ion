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
	 * @memcheck
	 */
	public function testSequenceAsPromise() {
		$seq = new Sequence(function ($x) {
			return $x + 10;
		});

		$this->promise(function () use ($seq) {
			$this->data["seq.result.1"] = yield $seq;
		}, false);

        $seq(1);
        $seq(2);

        $this->assertEquals([
            "seq.result.1" => 11
        ], $this->data);
	}

    /**
     * @group dev
     * @memcheck
     */
    public function testThenSequence() {
        $seq1 = new Sequence(function ($x) {
            return $x + 10;
        });
        $seq2 = new Sequence(function ($x) {
            return $x + 100;
        });
        $seq2->then(function ($x) {
            $this->data["x"] = $x;
        });

        $seq1->then($seq2);

        $seq1(1);

        $this->assertEquals([
            "x" => 111
        ], $this->data);
    }
}