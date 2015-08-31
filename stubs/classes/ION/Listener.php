<?php

namespace ION;


class Listener {

	public function __construct($socket) {}

	public function onConnect(callable $callback, $arg = null) {}

	public function awaitConnect() {}

	public function enable() {}

	public function disable() {}

	/**
	 *
	 */
	public function accept() {}

	public function shutdown() {}

	/**
	 * @return string
	 */
	public function __toString() {}

	public function __destruct() {}
}