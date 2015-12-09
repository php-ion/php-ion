<?php

namespace ION;


use ION\Test\TestCase;

class ServerTest extends TestCase {

    /**
     * @memcheck
     */
    public function testListen() {
        $server = new Server();

        $server->listen(ION_TEST_SERVER_IPV4);
        $server->listen(ION_TEST_SERVER_IPV6, 10);
    }
}