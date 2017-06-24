<?php

namespace ION;


use ION\Stream\StorageAbstract;

class Stream {
    const MODE_TRIM_TOKEN    = 1;
    const MODE_WITH_TOKEN    = 2;
    const MODE_WITHOUT_TOKEN = 4;

    const INPUT  = 2;
    const OUTPUT = 4;
    const BOTH   = self::INPUT | self::OUTPUT;

    /**
     * Create streams from resource of PHP stream.
     *
     * @param resource $resource
     *
     * @return self
     */
    public static function resource($resource) : self { }

    /**
     * Recognized resource formats are schema://hostname:port or path to unix socket:
     * www.example.com:80, 1.2.3.4:567, [::1]:8080, /var/run/service.sock
     *
     * @param string $host
     * @param Crypto $encrypt
     *
     * @return Stream
     *
     */
    public static function socket(string $host, Crypto $encrypt = null) : self { }

    /**
     * Create a pair of linked streams.
     * The streams behave as would two sockets connected to opposite ends of each other.
     *
     * @return self[]
     */
    public static function pair() : array { }

    /**
     * Activate stream for listening events
     *
     * @return self
     */
    public function enable() : self { }

    /**
     * Deactivate stream for listening events
     *
     * @return self
     */
    public function disable() : self { }

    /**
     * @todo
     * @return Deferred|self
     */
    public function connect() : Deferred { }

    /**
     * Assign a priority to a stream.
     * Only supported for socket streams.
     *
     * @param int $priority
     *
     * @return self
     */
    public function setPriority(int $priority) : self { }

    /**
     * If the input buffer is beyond the $bytes, the stream stops reading from the network. By default: unlimited
     *
     * @param int $bytes
     *
     * @return self
     */
    public function setInputMaxSize(int $bytes) : self { }

    /**
     * Starts encryption for current stream.
     *
     * @param Crypto $ssl
     *
     * @return self
     */
    public function encrypt(Crypto $ssl) : self { }

    /**
     * Queries the remote side of the given socket which may either result in host:port or in a Unix filesystem path,
     * dependent on its type
     * @return mixed
     */
    public function getPeerName() : string { }

    /**
     * Queries the local side of the given socket which may either result in host:port or in a Unix filesystem path,
     * dependent on its type
     * @return string
     */
    public function getLocalName() : string { }

    /**
     * Set the read and write timeouts for a Stream.
     * A Stream timeout will fire the first time that the indicated amount of time has elapsed since a successful
     * read or write operation, during which the Stream was trying to read or write.
     * In other words, if reading or writing is disabled, or if the stream's read or write operation has been suspended
     * because there's no data to write, or not enough bandwidth, or so on, the timeout isn't active.
     * The timeout only becomes active when we we're willing to actually read or write.
     *
     * @param float $read_timeout
     * @param float $write_timeout
     *
     * @return self
     */
//    public function setTimeouts(float $read_timeout, float $write_timeout) {}

    /**
     * Unique stream name
     * @return int
     */
    final public function getName() : int { }

    /**
     * Search for a string within an incoming buffer
     *
     * @param string $token the string to be searched for
     * @param int $offset   that indicates where we should stop searching
     * @param int $length   that indicates where we should start searching
     *
     * @return int|false the position of where the $token exists relative to the beginning of the incoming buffer
     */
    public function search(string $token, int $offset = 0, int $length = 0) : int { }

    /**
     * @param int $type
     *
     * @return int
     */
    public function getSize(int $type = self::INPUT) : int { }

    /**
     * Get N bytes
     *
     * @param int $bytes
     *
     * @return string|false
     */
    public function get(int $bytes) : string { }

    /**
     * Reads remainder of a stream into a string
     * @return string|false
     */
    public function getAll() : string { }

    /**
     * Gets line from input buffer up to a given delimiter
     *
     * @param string $token
     * @param int $flag
     * @param int $max_length
     *
     * @return string
     */
    public function getLine(string $token, int $flag = self::MODE_TRIM_TOKEN, int $max_length = 0) : string { }

    /**
     * @param int $count
     *
     * @return Deferred|string|false
     */
    public function read(int $count = 0) { }

    /**
     * @param string $token
     * @param int $flag
     * @param int $max_length
     *
     * @return Deferred|string|false
     */
    public function readLine(string $token, int $flag = self::MODE_TRIM_TOKEN, int $max_length = 8192) { }

    /**
     * Invoke callback when EOF received and read all data
     *
     * @return Deferred|string|false
     */
    public function readAll() { }

    /**
     * Write data to a stream.
     * The data is appended to the output buffer and written to the descriptor automatically as it becomes available
     * for writing.
     *
     * @param string $data
     *
     * @return Stream
     */
    public function write(string $data) : self { }

    /**
     * Copy data from a file into the output buffer for writing to a socket.
     * This method avoids unnecessary data copies between userland and kernel.
     *
     * @param string $filename path to file
     * @param int $offset      the offset from which to read data
     * @param int $limit       how much data to read
     *
     * @throws \RuntimeException if error occurs
     * @throws \InvalidArgumentException if passed invalid value
     *
     * @return self
     */
    public function sendFile(string $filename, int $offset = 0, int $limit = -1) { }

    /**
     * Done deferred object when outgoing buffer is empty
     *
     * @return Deferred|true
     */
    public function flush() { }

    /**
     *
     * @param bool $force close stream immediately
     *
     * @return Stream
     */
    public function shutdown(bool $force = false) { }

    /**
     * @return Sequence
     */
    public function incoming() : Sequence { }

    public function suspend() { }

    public function resume() { }

    /**
     * Deferred to be resolved when the stream is shutdown
     * @return Deferred $reason
     */
    public function closed() : Deferred { }

    /**
     * @return int
     */
    public function isClosed() : bool { }

    /**
     * @return bool
     */
    public function isConnected() : bool { }

    /**
     * @return bool
     */
    public function isEnabled() : bool { }

    /**
     * @return bool
     */
    public function hasEOF() : bool { }

    /**
     * Is stream errored
     * @return bool
     */
    public function hasError() : bool { }

    /**
     * @return \Exception
     */
    public function getError() : \Exception { }

    public function getType() : string { }

    /**
     * @return string
     */
    public function __toString() : string { }

    /**
     * @return array
     */
    public function __debugInfo() : array { }

    /**
     *
     */
    public function __destruct() { }

    /**
     * Append data to input buffer.
     * Available only in debug mode!
     *
     * @param string $data
     */
    public function appendToInput(string $data) : self { }

    /**
     *
     */
    private function _input() { }

    /**
     *
     */
    private function _output() { }

    /**
     *
     */
    private function _eof() { }

    private function _error() { }

    private function _open() { }

}