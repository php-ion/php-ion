<?php

namespace ION;

class PionTest extends TestCase {

	/**
	 * @group testCbCreate
	 * @memcheck
	 */
	public function testCbCreate() {
		$this->assertEquals(0, pionCbCreate(function ($a) {
			$this->testCbCreate = 1;
			return "retval";
		}, "test pionCbCreate"));
		$this->assertEquals(1, $this->testCbCreate);
	}

	/**
	 * @group testCbCreateFromZval
	 * @memcheck
	 * @deprecated
	 */
	public function _testCbCreateFromZval() {
		$this->assertEquals(0, pionCbCreateFromZval(function ($a) {
			$this->testCbCreateFromZval = 1;
			return "retval";
		}, "test pionCbCreateFromZval"));
		$this->assertEquals(1, $this->testCbCreateFromZval);
	}
}