<?php

namespace ION\Test;


use ION\Debug;

class DebugTest extends TestCase {

	public function providerFcallVoid() {
		return [
			[],
			[1],
			[1,2],
			[1,2,3],

		];
	}

	/**
	 * @group dev
	 * @dataProvider providerFcallVoid()
	 * @param mixed $arg1
	 * @param mixed $arg2
	 * @param mixed $arg3
	 */
	public function testFcallVoid($arg1 = null, $arg2 = null, $arg3 = null) {
		$callback = function () {
			return array_sum(func_get_args());
		};
		switch(func_num_args()) {
			case 0:
				Debug::fcallVoid($callback);
				return;
			case 1:
				Debug::fcallVoid($callback, $arg1);
				return;
			case 2:
				Debug::fcallVoid($callback, $arg1, $arg2);
				return;
			case 3:
				Debug::fcallVoid($callback, $arg1, $arg2, $arg3);
				return;
		}
	}
}