<?php

namespace ION;


class Listener {

	public function __construct($listen) {}

    /**
     * @param callable $action
     * @return Sequence
     */
	public function onConnect(callable $action) {}

    /**
     * @return self
     */
	public function enable() {}

    /**
     * @return self
     */
	public function disable() {}

    /**
     * @return self
     */
	public function shutdown() {}

	/**
	 * @return string
	 */
	public function __toString() {}

	public function __destruct() {}
}