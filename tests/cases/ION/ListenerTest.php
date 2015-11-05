<?php

namespace ION;


use ION\Test\TestCase;

class ListenerTest extends TestCase {

    /**
     *
     * @memcheck
     */
    public function testCreate() {
        $listener = new Listener("tcp://".ION_TEST_SERVER_HOST);
    }

    /**
     * @group dev
     * @memcheck
     */
    public function testAccept() {
        $listener = new Listener("tcp://".ION_TEST_SERVER_HOST);
        $listener->onConnect(function (Stream $connect) {
            $this->data["connect"] = $this->describe($connect);
            $this->data["remote"] = $connect->getRemotePeer();
            $this->data["local"] = $connect->getLocalName();
            $this->stop();
        })->onFail(function (\Throwable $error) {
            $this->data["error"] = $this->describe($error);
            $this->stop();
        });
        $client = stream_socket_client("tcp://".ION_TEST_SERVER_HOST, $errno, $error, 10, STREAM_CLIENT_ASYNC_CONNECT);
        stream_set_blocking($client, 0);
        $this->loop();

        $this->assertEquals([
            'object' => 'ION\Stream'
        ], $this->data['connect']);
        $this->assertArrayNotHasKey("error", $this->data);
        $this->assertEquals(ION_TEST_SERVER_HOST, $this->data["local"]);
        $this->assertStringMatchesFormat(parse_url(ION_TEST_SERVER_HOST, PHP_URL_HOST).":%i", $this->data["remote"]);
    }
}