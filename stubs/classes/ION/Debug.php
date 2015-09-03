<?php

namespace ION;


class Debug {

	/**
	 * @param callable $cb
	 * @param mixed $arg1
	 * @param mixed $arg2
	 * @param mixed $arg3
	 *
	 * @return int
	 */
	public static function fcallVoid(callable $cb, $arg1 = null, $arg2 = null, $arg3 = null) {}

	/**
	 * @param callable $cb
	 * @param mixed $arg1
	 * @param mixed $arg2
	 * @param mixed $arg3
	 *
	 * @return int
	 */
	public static function cbCallVoid(callable $cb, $arg1 = null, $arg2 = null, $arg3 = null) {}
}