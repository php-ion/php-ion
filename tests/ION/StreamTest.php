<?php

namespace ION;

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
}