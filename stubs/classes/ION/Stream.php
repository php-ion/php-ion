<?php

namespace ION;


use ION\Deferred\Map;

class Stream {
    const MODE_TRIM_TOKEN = 1;
    const MODE_WITH_TOKEN = 2;
    const MODE_WITHOUT_TOKEN = 4;

    const STATE_SOCKET = 1;
    const STATE_PAIR = 2;
    const STATE_PIPE = 4;

    const STATE_FLUSHED = 32;
    const STATE_HAS_DATA = 64;
    const STATE_CONNECTED = 1024;
    const STATE_EOF = 2048;
    const STATE_ERROR = 4096;
    const STATE_SHUTDOWN = 8192;
    const STATE_CLOSED = 14336;

    const NAME_HOST = 0;
    const NAME_ADDRESS = 1;
    const NAME_PORT = 2;

    const INPUT = 2;
    const OUTPUT = 4;
    const BOTH = 6;

	/**
	 * Create streams from resource of PHP stream.
	 *
	 * @param resource $resource
	 * @return self
	 */
	public static function resource($resource) {}

	/**
	 * Recognized host formats are hostname + port:
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
	 * If the input buffer is beyond the $bytes, the stream stops reading from the network.
	 * @param int $bytes
	 * @return self
	 */
	public function setInputSize($bytes) {}

	/**
	 * @todo
	 * @param mixed $ssl
	 * @return self
	 */
	public function ensureSSL($ssl) {}

    /**
     * Queries the remote side of the given socket which may either result in host:port or in a Unix filesystem path, dependent on its type
     * @return mixed
     */
	public function getRemotePeer() {}

    /**
     * Queries the local side of the given socket which may either result in host:port or in a Unix filesystem path, dependent on its type
     * @return string|false
     */
	public function getLocalName() {}

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
     * Reads remainder of a stream into a string
	 * @return string
	 */
	public function getAll() {}

	/**
     * Gets line from input buffer up to a given delimiter
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return string
	 */
	public function getLine($token, $flag = self::MODE_TRIM_TOKEN, $max_length = 0) {}

	/**
	 * @param int $count
	 *
	 * @return Deferred
	 */
	public function await($count = 0) {}

	/**
	 * @param string $token
	 * @param int $flag
	 * @param int $max_length
	 *
	 * @return Deferred
	 */
	public function awaitLine($token, $flag = self::MODE_TRIM_TOKEN, $max_length = 8192) {}

	/**
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
	 * @param string $filename path to file
	 * @param int $offset the offset from which to read data
	 * @param int $limit how much data to read
	 * @throws \RuntimeException if error occurs
	 * @throws \InvalidArgumentException if passed invalid value
	 *
	 * @return self
	 */
	public function sendFile($filename, $offset = 0, $limit = -1) {}

	/**
	 * Done deferred object when outgoing buffer is empty
	 *
	 * @return Deferred
	 */
	public function flush() {}

	/**
	 *
	 * @param bool $force close stream immediately
	 * @return Stream
	 */
	public function close($force = false) {}

    /**
     * @return Map
     */
	public function onData() {}

	/**
	 * Deferred to be resolved when the stream is shutdown
	 * @return Deferred
	 */
	public function awaitShutdown() {}

    /**
     * @return int
     */
    public function isClosed() {}

    /**
     * @return bool
     */
    public function isConnected() {}

    /**
     * @return bool
     */
    public function isEnabled() {}

    /**
     * @return bool
     */
    public function getState() {}

    /**
     * @return string
     */
    public function __toString() {}

    /**
     * @return array
     */
    public function __debugInfo() {}

    /**
     *
     */
	public function __destruct() {}

	/**
	 * Append data to input buffer.
	 * Available only in debug mode!
	 * @param string $data
	 */
	public function appendToInput($data) {}

}

class StreamException extends \Exception {}
class RuntimeException extends StreamException {}
class ConnectionException extends StreamException {}