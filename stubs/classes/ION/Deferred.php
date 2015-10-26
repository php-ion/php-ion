<?php
namespace ION;
/**
 * Ticket for deferred actions
 * @todo more doc
 */
class Deferred extends ResolvablePromise {

	/**
	 * @todo more doc
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