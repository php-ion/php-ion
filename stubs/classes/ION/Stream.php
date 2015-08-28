<?php

namespace ION;


class Stream {
	const MODE_TRIM_TOKEN    = 0b001;
	const MODE_WITH_TOKEN    = 0b010;
	const MODE_WITHOUT_TOKEN = 0b100;

	const INPUT  = 0b010;
	const OUTPUT = 0b100;
	const BOTH   = 0b110;

	/**
	 * Create streams from resource of PHP stream.
	 *
	 * @param resource $resource
	 * @return self
	 */
	public static function resource($resource) {}

	/**
	 * Recognized hostname formats are hostname + port:
	 * www.example.com:80, 1.2.3.4:567, [::1]:8080
	 * @param string $host
	 *
	 * @return self
	 */
	public static function socket($host) {}

	/**
	 * Create a pair of linked streams.
	 * The streams behave as would two sockets connected to opposite ends of each other.
	 *
	 * @return self[]
	 */
	public static function pair() {}

	/**
	 *
	 */
	private function _input() {}

	/**
	 *
	 */
	private function _output() {}

	/**
	 *
	 */
	private function _notify() {}

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
	 * @return string
	 */
	public function __toString() {}

	/**
	 * @todo
	 * @return Deferred
	 */
	public function awaitConnection() {}

	/**
	 * Set the read and write timeout for a Stream.
	 * A Stream timeout will fire the first time that the indicated amount of time has elapsed since a successful
	 * read or write operation, during which the Stream was trying to read or write.
	 * @param double $read_timeout
	 * @param double $write_timeout
	 * @return self
	 */
	public function setTimeouts($read_timeout, $write_timeout) {}

	/**
	 * Assign a priority to a stream.
	 * Only supported for socket streams.
	 * @param int $priority
	 * @return self
	 */
	public function setPriority($priority) {}

	/**
	 * @todo
	 * @param mixed $ssl
	 * @return self
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
	 * @param int $offset that indicates where we should stop searching
	 * @param int $length that indicates where we should start searching
	 *
	 * @return int|false the position of where the $token exists relative to the beginning of the incoming buffer
	 */
	public function search($token, $offset = 0, $length = 0) {}


	/**
	 * @param int $type
	 * @return int
	 */
	public function getSize($type = self::INPUT) {}

	/**
	 * Get N bytes
	 *
	 * @param int $bytes
	 * @return string
	 */
	public function get($bytes) {}

	/**
	 * @return string
	 */
	public function getAll() {}

	/**
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return string
	 */
	public function getLine($token, $flag = self::MODE_TRIM_TOKEN, $max_length = 0) {}

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
	public function awaitLine($token, $flag = self::MODE_TRIM_TOKEN, $max_length = 8192) {}

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

	public function __destruct() {}

	/**
	 * Append data to input buffer.
	 * Available only in debug mode!
	 * @param string $data
	 */
	public function appendToInput($data) {}

}