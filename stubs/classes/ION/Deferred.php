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
	 * @return self
	 */
	public function then(callable $cb) {}

	/**
	 * Cancel deferred object
	 * @param string $reason
	 */
	public function cancel($reason) {}

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
	 * Auto reject deferred object after N seconds
	 * @param int $seconds
	 */
	public function timeout($seconds) {}

	/**
	 * @todo
	 */
	public function getState() {}

	/**
	 * @todo
	 * @param mixed $info
	 */
	public function notify($info) {}

	/**
	 * @todo
	 * @param callable $cb
	 */
	public function done(callable $cb) {}

	/**
	 * @todo
	 * @param callable $cb
	 */
	public function fail(callable $cb) {}
	/**
	 * @todo
	 * @param callable $cb
	 */
	public function progress(callable $cb) {}
}

class CancelException extends \Exception {}
class TimeoutException extends CancelException {}