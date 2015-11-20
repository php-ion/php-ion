<?php

namespace ION;

/**
 * Ticket for deferred actions
 */
class Deferred extends ResolvablePromise {

	/**
	 * @param callable $canceler
	 */
	public function __construct(callable $canceler = null) {}


	/**
	 * Cancel deferred object
	 * @param string $reason
	 */
	public function cancel($reason) {}


    public function __clone() {}
}