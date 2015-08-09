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
	 */
	public function __construct(callable $cancel_cb = null) {}
	/**
	 * Set user callback on finish deferred-event
	 * @param callable $cb
	 */
	public function then(callable $cb) {}

	/**
	 * Cancel deferred object
	 * @param string $reason
	 */
	public function reject($reason) {}

	/**
	 * Successfully complete the deferred-event
	 * @param mixed $data
	 */
	public function resolve($data) {}

	/**
	 * @todo more doc
	 * @param \Exception $error
	 */
	public function error(\Exception $error) {}

	/**
	 * Set timeout in seconds for deferred object. After timeout defer will be canceled.
	 * @param int $seconds
	 */
	public function timeout($seconds) {}

	/**
	 * Processing internal deferred-object queue
	 * @internal this is important method invoks after each event
	 */
	public static function dequeue() {}
}

class CancelException extends \Exception {}
class TimeoutException extends CancelException {}