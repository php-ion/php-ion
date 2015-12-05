<?php

namespace ION;


use ION\Test\TestCase;

class CryptoTest extends TestCase {

    /**
     * @memcheck
     */
    public function testFactories() {
        $client = Crypto::client();
        $server = Crypto::server();
    }

    /**
     * @memcheck
     */
    public function testLoadLocalCert() {
        Crypto::server()->allowSelfSigned()->verifyPeer()->compression(false)->ticket(false)->localCert(ION_RESOURCES.'/cert', ION_RESOURCES.'/pkey');
    }

    /**
     * @memcheck
     */
    public function testLoadCA() {
        Crypto::server()->ca(ION_RESOURCES.'/cacert.pem');

    }
}