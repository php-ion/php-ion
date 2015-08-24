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
	 * Activate stream for listening events
	 *
	 * @return self
	 */
	public function enable() {}

	/**
	 * Deactivate stream for listening events
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
	 * Search for a string within an incoming buffer
	 *
	 * @param string $token the string to be searched for
	 * @param int $length that indicates where we should start searching
	 * @param int $offset that indicates where we should stop searching
	 *
	 * @return int|false the position of where the $token exists relative to the beginning of the incoming buffer
	 */
	public function search($token, $length = 0, $offset = 0) {}


	/**
	 * @param int $type
	 */
	public function getSize($type = \ION::READ) {}

	/**
	 * Get N bytes
	 *
	 * @param int $count
	 * @return string
	 */
	public function read($count = -1) {}

	/**
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return string
	 */
	public function readLine($token, $flag = self::WITHOUT_TOKEN, $max_length = 8192) {}

	/**
	 * @todo
	 * @param int $count
	 *
	 * @return Deferred
	 */
	public function await($count = 0) {}

	/**
	 * @todo
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return Deferred
	 */
	public function awaitLine($token, $flag = self::WITHOUT_TOKEN, $max_length = 8192) {}

	/**
	 * @todo
	 * Invoke callback when EOF received and read all data
	 *
	 * @return Deferred
	 */
	public function awaitAll() {}

	/**
	 * Write data to a stream.
	 * The data is appended to the output buffer and written to the descriptor automatically as it becomes available for writing.
	 *
	 * @param string $data
	 *
	 * @return self
	 */
	public function write($data) {}

	/**
	 * Copy data from a file descriptor into the event buffer for writing to a socket.
	 * This method avoids unnecessary data copies between userland and kernel.
	 *
	 * @param resource $fd the file descriptor
	 * @param int $offset the offset from which to read data
	 * @param int $limit how much data to read
	 * @throws \RuntimeException if error occurs
	 * @throws \InvalidArgumentException if passed invalid value
	 *
	 * @return self
	 */
	public function sendFile($fd, $offset = 0, $limit = -1) {}

	/**
	 * Done deferred object when outgoing buffer is empty
	 *
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