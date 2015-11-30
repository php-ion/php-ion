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
     * @group dev
     */
    public function testEncrypt() {
        $dn = array(
            "countryName" => "UK",
            "stateOrProvinceName" => "Somerset",
            "localityName" => "Glastonbury",
            "organizationName" => "The Brain Room Limited",
            "organizationalUnitName" => "PHP Documentation Team",
            "commonName" => "Wez Furlong",
            "emailAddress" => "wez@example.com"
        );
        // Generate certificate
        $privkey = openssl_pkey_new();
        $cert    = openssl_csr_new($dn, $privkey);
        $cert    = openssl_csr_sign($cert, null, $privkey, 365);
        // Generate PEM file
        # Optionally change the passphrase from 'comet' to whatever you want, or leave it empty for no passphrase
        $pem_passphrase = 'testing';
        $pem = array();
        openssl_x509_export($cert, $pem[0]);
        openssl_pkey_export($privkey, $pem[1], $pem_passphrase);
        $pem = implode($pem);
        $pemfile = ION_VAR.'/server.pem';
        file_put_contents($pemfile, $pem);

        $listener = new Listener(ION_TEST_SERVER_HOST);
        $listener->encrypt(
            SSL::server(SSL::METHOD_TLSv12)->passPhrase($pem_passphrase)->localCert($pemfile)->allowSelfSigned()
        );
        $listener->accept()->then(function (Stream $connect) {
            $this->data["connect"] = $this->describe($connect);
            $this->data["incoming"] = yield $connect->readLine("\r\n");
            $connect->write("welcome\r\n");
            yield $connect->flush();
            $this->stop();
        })->onFail(function (\Throwable $error) {
            $this->data["server.error"] = $this->describe($error);
            $this->stop();
        });

        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_HOST, SSL::client()->allowSelfSigned());
            $socket->write("hello\r\n");
            $this->data["outgoing"] = yield $socket->readLine("\r\n");
        }, false);

        $this->loop();

        $this->out($this->data);
    }
}