<?php

namespace ION;

/**
 * For internal testing
 * @package ION
 */
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

    /**
     * @param mixed $arg
     *
     * @return mixed
     */
    public static function globalCbCall($arg) {}

    /**
     * @param callable $cb
     *
     * @return mixed
     */
    public static function globalCbCreate(callable $cb) {}

    /**
     * @param callable $cb
     *
     * @return mixed
     */
    public static function globalCbCreateFromZval(callable $cb) {}
}