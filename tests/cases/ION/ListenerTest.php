<?php

namespace ION;


use ION\Test\Conn;
use ION\Test\TestCase;

class ListenerTest extends TestCase {

    /**
     * @memcheck
     */
    public function testCreate() {
        $listener = new Listener(ION_TEST_SERVER_IPV4, 10);
        $listener = new Listener(ION_TEST_SERVER_IPV6, -1);
        $listener = new Listener(ION_TEST_SERVER_UNIX, -1);
    }

    public function providerHosts() {
        return [
            [ION_TEST_SERVER_IPV4, "", ""],
            [ION_TEST_SERVER_IPV6, "", ""],
            [ION_TEST_SERVER_UNIX, "unix://".ION_TEST_SERVER_UNIX, ""],

            [ION_TEST_SERVER_IPV4, "", Conn::class],
            [ION_TEST_SERVER_IPV6, "", Conn::class],
            [ION_TEST_SERVER_UNIX, "unix://".ION_TEST_SERVER_UNIX, Conn::class],
        ];
    }

    /**
     * @dataProvider providerHosts
     * @memcheck
     *
     * @param string $address
     * @param string $stream_address
     * @param $class
     */
    public function testAccept($address, $stream_address, $class) {

        $listener = new Listener($address);
        if($class) {
            $listener->setStreamClass($class);
        }
        $listener->whenAccepted()->then(function (Stream $connect) use ($listener) {
            $this->data["connect"] = $this->describe($connect);
            $this->stop();
            $listener->shutdown();
        })->onFail(function (\Throwable $error) use ($listener) {
            $this->data["error"] = $this->describe($error);
            $this->stop();
            $listener->shutdown();
        });
        $client = stream_socket_client($stream_address?:$address, $errno, $error, 10, STREAM_CLIENT_ASYNC_CONNECT);
        stream_set_blocking($client, 0);

        $this->loop();
        stream_socket_shutdown($client, STREAM_SHUT_RDWR);
        unset($client);
        if($stream_address) {
            unlink($address);
        }

        $this->assertEquals([
            'object' => $class ?: Stream::class
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
    public function testEncrypt() {

        $listener = new Listener(ION_TEST_SERVER_HOST);
        $server_ssl = Crypto::server(Crypto::METHOD_TLSv12)
            ->passPhrase('unittest')
            ->localCert(ION_RESOURCES.'/cacert.pem', ION_RESOURCES.'/cakey.pem')
            ->allowSelfSigned();

        $client_ssl = Crypto::client(Crypto::METHOD_TLSv12)
            ->allowSelfSigned();

        $listener->encrypt($server_ssl);
        $listener->whenAccepted()->then(function (Stream $connect) {
            $this->data["connect"] = $this->describe($connect);
            $this->data["incoming"] = yield $connect->readLine("\r\n");

            $connect->write("welcome\r\n");
            yield $connect->flush();
            $this->stop();
        })->onFail(function (\Throwable $error) {
            $this->data["server.error"] = $this->describe($error);
            $this->stop();
        });

        $this->promise(function () use ($client_ssl) {
            $socket = Stream::socket(ION_TEST_SERVER_HOST, $client_ssl);
            $socket->write("hello\r\n");
            $this->data["outgoing"] = yield $socket->readLine("\r\n");
        }, false);

        $this->loop();

        $this->assertEquals([
            "connect" => [
                "object" => "ION\\Stream"
            ],
            "incoming" => "hello",
            "outgoing" => "welcome"
        ], $this->data);
    }
}