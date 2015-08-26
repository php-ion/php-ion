<?php

namespace ION;

use ION;

class StreamTest extends TestCase {

	/**
	 * @memcheck
	 */
	public function testCreatePair() {
		$pair = Stream::pair();
		$this->assertCount(2, $pair);
		$this->assertInstanceOf('ION\Stream', $pair[0]);
		$this->assertInstanceOf('ION\Stream', $pair[1]);
	}

	/**
	 * @memcheck
	 */
	public function testCreateFromResource() {
		$stream = Stream::resource(STDIN);
		$this->assertInstanceOf('ION\Stream', $stream);
	}

	/**
	 * @memcheck
	 */
	public function testSocket() {
		$pid = $this->listen(ION_TEST_SERVER_HOST)->inWorker()->onConnect(function ($connect) {
//			$this->out("Accept connection");
		})->start();

		$stream = Stream::socket(ION_TEST_SERVER_HOST);
		$this->assertInstanceOf('ION\Stream', $stream);
	}

	/**
	 * @memcheck
	 */
	public function testEnableDisable() {
		list($a, $b) = Stream::pair();
		/* @var Stream $a */
		/* @var Stream $b */
		$a->enable()->disable()->enable();
		$b->enable()->disable()->enable();
	}

	public function providerGets() {
		$string = "0123456789";
		return array(
			0  => [$string, "get", [5],                                        "01234", "56789"],
			1  => [$string, "get", [10],                                       "0123456789", ""],
			2  => [$string, "get", [16],                                       "0123456789", ""],

			3  => [$string, "getAll",  [],                                     $string, ""],

			4  => [$string, "getLine", ["3", Stream::MODE_TRIM_TOKEN, 8],      "012", "456789"],
			5  => [$string, "getLine", ["3", Stream::MODE_WITH_TOKEN, 8],      "0123", "456789"],
			6  => [$string, "getLine", ["3", Stream::MODE_WITHOUT_TOKEN, 8],   "012", "3456789"],

			7  => [$string, "getLine", ["3", Stream::MODE_TRIM_TOKEN, 18],     "012", "456789"],
			8  => [$string, "getLine", ["3", Stream::MODE_WITH_TOKEN, 18],     "0123", "456789"],
			9  => [$string, "getLine", ["3", Stream::MODE_WITHOUT_TOKEN, 18],  "012", "3456789"],

			10 => [$string, "getLine", ["9", Stream::MODE_TRIM_TOKEN, 4],      false, $string],
			11 => [$string, "getLine", ["9", Stream::MODE_WITH_TOKEN, 4],      false, $string],
			12 => [$string, "getLine", ["9", Stream::MODE_WITHOUT_TOKEN, 4],   false, $string],

			13 => [$string, "getLine", ["01", Stream::MODE_TRIM_TOKEN, 8],     "", "23456789"],
			14 => [$string, "getLine", ["01", Stream::MODE_WITH_TOKEN, 8],     "01", "23456789"],
			15 => [$string, "getLine", ["01", Stream::MODE_WITHOUT_TOKEN, 8],  "", "0123456789"],

			16 => [$string, "getLine", ["89", Stream::MODE_TRIM_TOKEN, 10],    "01234567", ""],
			17 => [$string, "getLine", ["89", Stream::MODE_WITH_TOKEN, 10],    "0123456789", ""],
			18 => [$string, "getLine", ["89", Stream::MODE_WITHOUT_TOKEN, 10], "01234567", "89"],

			19 => [$string, "getLine", ["45", Stream::MODE_TRIM_TOKEN],        "0123", "6789"],
			20 => [$string, "getLine", ["45", Stream::MODE_WITH_TOKEN],        "012345", "6789"],
			21 => [$string, "getLine", ["45", Stream::MODE_WITHOUT_TOKEN],     "0123", "456789"],
		);
	}

	/**
	 * @group dev
	 * @memcheck
	 * @dataProvider providerGets
	 */
	public function testGets($string, $method, $args, $result, $tail) {
		$pid = $this->listen(ION_TEST_SERVER_HOST)->inWorker()->onConnect(function ($connect) use ($string) {
			fwrite($connect, $string);
		})->start();

		$socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();
		ION::await(0.03)->then(function () use($socket, $method, $args) {
			$this->data[$method] = call_user_func_array([$socket, $method], $args);
			$this->data["tail"]  = $socket->getAll();
			$this->stop();
		});
		$this->loop();

		$this->assertEquals([
			$method   => $result,
			"tail"    => $tail
		], $this->data);
		$this->assertWaitPID($pid);
	}
}