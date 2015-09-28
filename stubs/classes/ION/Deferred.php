<?php
namespace ION;
/**
 * Ticket for deferred actions
 * @todo more doc
 */
class Deferred {

	/**
	 * @todo more doc
	 * @param callable $canceler
	 */
	public function __construct(callable $canceler = null) {}

    /**
     * Set user callback on finish deferred-event
     * @param callable $done
     * @param callable $fail
     * @param callable $progress
     * @return Promise
     */
	public function then(callable $done = null, callable $fail = null, callable $progress = null) {}

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
     * @return self
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
     * @return Promise
	 */
	public function done(callable $cb) {}

	/**
	 * @todo
	 * @param callable $cb
     * @return Promise
	 */
	public function fail(callable $cb) {}
	/**
	 * @todo
	 * @param callable $cb
     * @return self
	 */
	public function progress(callable $cb) {}

    public function __clone() {}
}

class CancelException extends \Exception {}
class TimeoutException extends CancelException {}