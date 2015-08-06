<?php
namespace ION;
/**
 * Ticket for deferred actions
 * @todo more doc
 */
final class Deferred {

	/**
	 * @todo more doc
	 * @param callable $cancel_cb
	 * @param mixed $cancel_arg
	 */
	public function __construct(callable $cancel_cb = null) {}
	/**
	 * Set user callback on finish defer-event
	 * @param callable $cb
	 * @param mixed $arg
	 */
	public function then(callable $cb) {}

	/**
	 * Cancel defer object
	 */
	public function reject() {}

	/**
	 * Successfully complete the defer-event
	 * @param mixed $data
	 */
	public function resolve($data) {}

	/**
	 * @todo more doc
	 * @param Exception $error
	 */
	public function error(Exception $error) {}

	/**
	 * Set timeout in seconds for defer object. After timeout defer will be canceled.
	 * @param int $seconds
	 */
	public function timeout($seconds) {}

	/**
	 * Processing internal defer-object queue
	 * @internal this is important method invoks after each event
	 */
	public static function dequeue() {}
}

class CancelException extends Exception {}
class TimeoutException extends CancelException {}