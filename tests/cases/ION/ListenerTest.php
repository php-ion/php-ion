<?php

namespace ION;


use ION\Test\TestCase;

class ListenerTest extends TestCase {

    /**
     *
     * @memcheck
     */
    public function testCreate() {
        $listener = new Listener("tcp://".ION_TEST_SERVER_IPV4);
        $listener = new Listener("tcp://".ION_TEST_SERVER_IPV6);
    }

    public function providerHosts() {
        return [
            ["tcp://".ION_TEST_SERVER_IPV4, ION_TEST_SERVER_IPV4],
            ["tcp://".ION_TEST_SERVER_IPV6, ION_TEST_SERVER_IPV6],
//            [ION_TEST_SERVER_UNIX, ION_TEST_SERVER_UNIX, "unix://".ION_TEST_SERVER_UNIX], // TODO: fix ION::stop() when unix listening
        ];
    }

    /**
     * @dataProvider providerHosts
     * @memcheck
     * @param string $address
     * @param string $stream_address
     */
    public function testAccept($address, $name, $stream_address = null) {

        $listener = new Listener($address);
        $listener->onConnect(function (Stream $connect) {
            $this->data["connect"] = $this->describe($connect);
//            $this->data["remote"] = $connect->getRemotePeer();
//            $this->data["local"] = $connect->getLocalName();

            $this->stop();
        })->onFail(function (\Throwable $error) {
            $this->data["error"] = $this->describe($error);
            $this->stop();
        });
        $client = stream_socket_client($stream_address?:$address, $errno, $error, 10, STREAM_CLIENT_ASYNC_CONNECT);
        stream_set_blocking($client, 0);

        $this->loop();

        $this->assertEquals([
            'object' => 'ION\Stream'
        ], $this->data['connect']);
        $this->assertArrayNotHasKey("error", $this->data);
//        $this->assertEquals($name, $this->data["local"]);
//        $this->assertStringMatchesFormat(parse_url($name, PHP_URL_HOST).":%i", $this->data["remote"]);
    }

    /**
     * @memcheck
     */
    public function testEnableDisable() {
        $listener = new Listener("tcp://".ION_TEST_SERVER_HOST);
        $listener->disable()->enable();
    }

    /**
     * @memcheck
     */
    public function testShutDown() {
        $listener = new Listener("tcp://".ION_TEST_SERVER_HOST);
        $listener->shutdown();
    }

    /**
     * @memcheck
     */
    public function testToString() {
        $listener = new Listener("tcp://".ION_TEST_SERVER_HOST);
       $this->assertEquals(ION_TEST_SERVER_HOST, strval($listener));
    }
}