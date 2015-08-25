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
		$stream = Stream::socket("www.example.com:80");
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

	/**
	 * @group dev
	 * @memcheck
	 */
	public function testGet() {
		$pid = $this->startWorker(function () {
			$this->listen(ION_TEST_SERVER_HOST)->onConnect(function ($connect) {
				$this->out("Accept connection");
				$this->out("Sent: ".fwrite($connect, "0123456789"));
			})->start();
		}, -0.1);

		$socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();
		ION::await(0.1)->then(function () use($socket) {
			$this->data["get"] = $socket->get(5);
			$this->stop();
		});
		$this->loop(1);

		$this->assertEquals([
			"get" => "01234"
		], $this->data);
		$this->assertWaitPID($pid);
	}
}