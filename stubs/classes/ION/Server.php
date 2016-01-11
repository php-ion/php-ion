<?php

namespace ION;


use ION\Stream\Connection;

/**
 * CONCEPT.
 * @package ION
 */
class Server {

    /**
     * Create an Internet or Unix domain server socket
     *
     * @param string $host
     * @param int $backlog
     *
     * @return Listener
     */
    public function listen(string $host, int $backlog = -1) : Listener { }

    /**
     * Shutdown an Internet or Unix domain server socket
     *
     * @param string $host server socket name after creation
     *
     * @return Server
     */
    public function shutdownListener(string $host) : self { }

    /**
     * Set maximum connections
     *
     * @param int $max
     *
     * @return Server
     */
    public function setMaxConsLimit(int $max) : self { }

    /**
     * Set idle connection timout.
     * After timeout timeout sequence will be invoked.
     *
     * @param int $sec
     *
     * @return Server
     */
    public function setIdleTimeout(int $sec) : self { }

    public function setUseTimeout(int $sec) : self { }

    public function setReadLimits(int $rate, int $burst) : self { }

    public function setWriteLimits(int $rate, int $burst) : self { }

    public function setPriority(int $priority) : self { }

    public function setInputBufferSize(int $size) : self { }

    public function setPingTimeout(int $sec) : self { }

    /**
     * Sequence for new connection
     * @return Sequence
     */
    public function accept() : Sequence { }

    public function incoming() : Sequence { }

    public function timeout() : Sequence { }

    public function close() : Sequence { }

    public function ping() : Sequence { }

    public function getConnection($peer_name) : Stream { }

    public function hasConnection($peer_name) : bool { }

    public function count() : int { }

    public function getTotalRead() : int { }

    public function getTotalWritten() : int { }

    public function getTotalAccepted() : int { }

    public function resetTotals() : self { }

    public function shutdown($host) : self { }

    public function __debugInfo() : array { }
}

$server = new Server();

$server->listen("0.0.0.0:8080", 100)->encrypt(Crypto::server());
$server->incoming()->then(function (Connection $connect) {
    if (!$headers = yield $connect->readLine("\r\n\r\n", Stream::MODE_TRIM_TOKEN, 64 * KiB)) {
        $connect->write("HTTP/1.0 403 Bad Request\r\n\r\n");
    }
    // parse $headers


    $connect->free(); // send connect to pool
});
$server->timeout()->then(function (Connection $connect) {
    if (!$connect->isFree()) {
        // do reject
    }
    $connect->close();
});