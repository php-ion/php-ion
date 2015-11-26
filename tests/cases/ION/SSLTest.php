<?php

namespace ION;


use ION\Test\TestCase;

class SSLTest extends TestCase {

    /**
     * @memcheck
     */
    public function testFactories() {
        $client = SSL::client();
        $server = SSL::server();
    }

    /**
     * @memcheck
     */
    public function testLoadLocalCert() {
        SSL::server()->allowSelfSigned()->verifyPeer()->compression(false)->ticket(false)->localCert(ION_RESOURCES.'/cert', ION_RESOURCES.'/pkey');
    }

    /**
     * @memcheck
     */
    public function testLoadCA() {
        SSL::server()->ca(ION_RESOURCES.'/cacert.pem');

    }
}