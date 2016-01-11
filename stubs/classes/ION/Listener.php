<?php

namespace ION;

/**
 * This class gives you a way to listen for and accept incoming TCP connections.
 * example.com:80, 127.0.0.1:8080, [::1]:8081, /tmp/server.sock
 * @package ION
 */
class Listener {

    /**
     * Listens for a connection on a socket
     *
     * @param string $listen socket name HOST:PORT or unix domain socket
     * @param int $backlog   Set the maximum number of pending connections that the network stack should allow to wait
     *                       in a not-yet-accepted state at any time; see documentation for your system’s listen()
     *                       function for more details. If backlog is negative, Libevent tries to pick a good value for
     *                       the backlog.
     */
    public function __construct(string $listen, int $backlog = -1) { }

    /**
     * @param Crypto $ssl
     *
     * @return Listener
     */
    public function encrypt(Crypto $ssl) : self { }

    /**
     * When a new connection is received, the sequence is invoked with new connection.
     *
     * @return Sequence
     */
    public function accept() : Sequence { }

    /**
     * Reenable listener for new connections.
     *
     * @return self
     */
    public function enable() : self { }

    /**
     * Temporarily disable listener for new connections.
     *
     * @return self
     */
    public function disable() : self { }

    /**
     * @return self
     */
    public function shutdown() : self { }

    /**
     *
     */
    public function getName() : string { }

    /**
     * @return string
     */
    public function __toString() : string { }

    public function __destruct() { }
}