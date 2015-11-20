<?php

namespace ION;


use ION\Test\TestCase;

class FSTest extends TestCase {

	/**
	 * @memcheck
	 */
	public function testReadFile() {
		$this->promise(function () {
			$this->data['content'] = yield FS::readFile(__FILE__, 0, 100 * MiB);
		});
		$this->loop();
		$this->assertStringEqualsFile(__FILE__, $this->data['content']);
	}

	/**
	 * @memcheck
	 */
	public function testWatch() {
		$file = "/tmp/iddqd";
		@unlink($file);
		file_put_contents($file, "a1");
		$expected = realpath($file);
		FS::watch($file, FS::WATCH_MODIFY)->then(function($filename) {
			$this->data["changed"] = $filename;
		});
		$this->promise(function () use ($file) {
			yield \ION::await(0.02);
			@unlink($file);
//			file_put_contents('/tmp/iddqd', "a2", FILE_APPEND);
			yield \ION::await(0.1);

		});
		$this->loop();
		FS::unwatchAll();
		$this->assertEquals([
			"changed" => $expected
		], $this->data);
	}
}