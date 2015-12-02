<?php

namespace ION;


use ION\Test\TestCase;

class FSTest extends TestCase {

	/**
     * @group dev
	 * @mem check
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
	public function _testWatch() {
		$file = "/tmp/iddqd";
		@unlink($file);
		file_put_contents($file, "a1");
		$expected = realpath($file);
		FS::watch($file)->then(function($filename) {
			$this->data["changed"] = $filename;
		})->setName('test');
		$this->promise(function () use ($file) {
			yield \ION::await(0.02);
			@unlink($file);
//			file_put_contents('/tmp/iddqd', "a2", FILE_APPEND);
			yield \ION::await(0.1);

		});
		$this->loop();
		FS::unwatchAll();
//        FS::watch($file)->forget('test');
		$this->assertEquals([
			"changed" => $expected
		], $this->data);
	}
}