<?php

namespace ION;


use ION\Test\TestCase;

class ListenerTest extends TestCase {

    /**
     * @memcheck
     */
    public function testCreate() {
        $listener = new Listener(ION_TEST_SERVER_IPV4, 10);
        $listener = new Listener(ION_TEST_SERVER_IPV6, -1);
    }

    public function providerHosts() {
        return [
            [ION_TEST_SERVER_IPV4, ION_TEST_SERVER_IPV4],
            [ION_TEST_SERVER_IPV6, ION_TEST_SERVER_IPV6],
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
        $listener->accept()->then(function (Stream $connect) {
            $this->data["connect"] = $this->describe($connect);
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
    }

    /**
     * @memcheck
     */
    public function testEnableDisable() {
        $listener = new Listener(ION_TEST_SERVER_HOST);
        $listener->disable()->enable();
    }

    /**
     * @memcheck
     */
    public function testShutDown() {
        $listener = new Listener(ION_TEST_SERVER_HOST);
        $listener->shutdown();
    }

    /**
     * @memcheck
     */
    public function testToString() {
        $listener = new Listener(ION_TEST_SERVER_HOST);
        $this->assertEquals(ION_TEST_SERVER_HOST, strval($listener));
    }

    /**
     * @memcheck
     */
    public function _testSSL() {
        $listener = new Listener(ION_TEST_SERVER_HOST);
        $listener->setSSL(
            SSL::server()->localCert(ION_RESOURCES.'/cert', ION_RESOURCES.'/pkey')->allowSelfSigned()
        );
        $listener->accept()->then(function (Stream $connect) {
            $this->data["connect"] = $this->describe($connect);
            $connect->write("HELLO");
            yield $connect->flush();
            $this->stop();
        })->onFail(function (\Throwable $error) {
            $this->data["error"] = $this->describe($error);
            $this->stop();
        });

        $client = stream_socket_client("ssl://".ION_TEST_SERVER_HOST, $errno, $error, 10, STREAM_CLIENT_ASYNC_CONNECT);
        stream_set_blocking($client, 0);

        $this->loop();
    }
}