<?php

namespace ION;


use ION\Test\TestCase;

class ServerTest extends TestCase {

    /**
     * @memcheck
     */
    public function testListeners() {
        $server = new Server();

        $this->assertInstanceOf(Listener::class, $server->listen(ION_TEST_SERVER_IPV4));
        $this->assertInstanceOf(Listener::class, $server->listen(ION_TEST_SERVER_IPV6, 10));

        $server->shutdown(ION_TEST_SERVER_IPV4);
        $server->shutdown(ION_TEST_SERVER_IPV6);
    }

    /**
     * @group dev
     */
    public function testAccept() {

        $server = new Server();

        $server->accept()->then(function (Stream $stream) {

        });
    }
}