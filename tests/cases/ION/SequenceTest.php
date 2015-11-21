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
     * @memcheck
     */
    public function testSequenceThenSequence() {
        $this->data["var"] = "x";

        $seq1 = new Sequence(function ($x) {
            return $x + 10;
        });
        $seq2 = new Sequence(function ($x) {
            return $x + 100;
        });
        $seq2->then(function ($x) {
            $this->data[ $this->data["var"] ] = $x;
        });

        $seq1->then($seq2);

        $seq1(1);
        $this->data["var"] = "y";
        $seq1(2);
        $this->data["var"] = "z";
        $seq2(3);

        $this->assertEquals([
            "var" => "z",
            "x" => 111,
            "y" => 112,
            "z" => 103
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testPromiseThen() {
        $this->data["var"] = "x";
        $promise = new ResolvablePromise(function ($x) {
            return $x + 10;
        });
        $seq2 = new Sequence(function ($x) {
            return $x + 100;
        });
        $seq2->then(function ($x) {
            $this->data[ $this->data["var"] ] = $x;
        });

        $promise->then($seq2);

        $promise->done(1);
        $this->data["var"] = "y";
        $seq2(2);

        $this->assertEquals([
            "var" => "y",
            "x" => 111,
            "y" => 102,
        ], $this->data);


    }

    /**
     * @memcheck
     */
    public function testSequenceThenPromise() {
        $promise = new ResolvablePromise(function ($x) {
            return $x + 10;
        });
        $seq = new Sequence(function ($x) {
            return $x + 100;
        });
        $seq->then($promise)->then(function ($x) {
            $this->data["x"] = $x;
        });

        $seq(1);
        $seq(2);

        $this->assertEquals([
            "x" => 111,
        ], $this->data);
    }
}