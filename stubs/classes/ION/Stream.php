<?php

namespace ION;


class Stream {
	const WITH_TOKEN    = 1;
	const WITHOUT_TOKEN = 2;
	const TRIM_TOKEN    = 3;

	/**
	 * @param resource $resource
	 *
	 * @return Stream
	 */
	public static function resource($resource) {}

	/**
	 * Recognized hostname formats are hostname + port:
	 * www.example.com:80, 1.2.3.4:567, [::1]:8080
	 * @param string $host
	 *
	 * @return Deferred
	 */
	public static function socket($host) {}

	/**
	 * @return Stream[]
	 */
	public static function pair() {}

	/**
	 * Enable stream
	 *
	 * @return self
	 */
	public function enable() {}

	/**
	 * Disable stream
	 *
	 * @return self
	 */
	public function disable() {}

	/**
	 * @todo
	 * @return string
	 */
	public function __toString() {}

	/**
	 * @todo
	 * @return Deferred
	 */
	public function awaitConnect() {}

	/**
	 * Set the read and write timeout for a Stream.
	 * A Stream timeout will fire the first time that the indicated amount of time has elapsed since a successful
	 * read or write operation, during which the Stream was trying to read or write.
	 * @param double $read_timeout
	 * @param double $write_timeout
	 */
	public function setTimeouts($read_timeout, $write_timeout) {}

	/**
	 * Assign a priority to a stream.
	 * Only supported for socket streams.
	 * @param int $priority
	 */
	public function setPriority($priority) {}

	/**
	 * @todo
	 * @param mixed $ssl
	 */
	public function ensureSSL($ssl) {}

	/**
	 * @todo
	 * @return string
	 */
	public function getRemotePeer() {}

	/**
	 * @todo
	 * @return string
	 */
	public function getLocalPeer() {}

	/**
	 * @todo
	 * @param string $token
	 * @param int $max_length
	 * @param int $offset
	 *
	 * @return int|false
	 */
	public function getPositionOf($token, $max_length = -1, $offset = 0) {}


	/**
	 * @todo
	 * @param int $type
	 */
	public function getLength($type = \ION::READ) {}

	/**
	 * @todo
	 * Get N bytes
	 *
	 * @param int $count
	 * @return string
	 */
	public function get($count = 0) {}

	/**
	 * @todo
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return string
	 */
	public function getLine($token, $flag = self::WITHOUT_TOKEN, $max_length = 8192) {}

	/**
	 * @todo
	 * @param int $count
	 *
	 * @return Deferred
	 */
	public function fetch($count = 0) {}

	/**
	 * @todo
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return Deferred
	 */
	public function fetchLine($token, $flag = self::WITHOUT_TOKEN, $max_length = 8192) {}

	/**
	 * @todo
	 * Invoke callback when EOF received and read all data
	 *
	 * @return Deferred
	 */
	public function fetchAll() {}

	/**
	 * @param string $data
	 *
	 * @return self
	 */
	public function write($data) {}

	/**
	 * @todo
	 * @param resource $fd
	 * @param int $offset
	 * @param int $limit
	 *
	 * @return self
	 */
	public function sendFile($fd, $offset = 0, $limit = -1) {}

	/**
	 * @todo
	 * @return Deferred
	 */
	public function flush() {}

	/**
	 * @todo
	 * @return Deferred
	 */
	public function close() {}

	/**
	 * @todo
	 * Close socket immediately
	 *
	 * @return Stream
	 */
	public function shutdown() {}

	public function onData(callable $cb) {}

	public function onClose(callable $cb) {}

}