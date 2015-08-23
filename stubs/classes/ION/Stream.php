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
	 * @param string $host
	 *
	 * @return Deferred
	 */
	public static function connect($host) {}

	/**
	 * @return Stream[]
	 */
	public static function pair() {}

	/**
	 * @todo
	 * Enable stream
	 *
	 * @return self
	 */
	public function enable() {}

	/**
	 * @todo
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
	 * @param int $timeout
	 */
	public function setWriteTimeout($timeout) {}

	/**
	 * @todo
	 * @param $type
	 * @param $priority
	 */
	public function setPriority($type, $priority) {}

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
	 * @todo
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
	 * @return Deferred
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