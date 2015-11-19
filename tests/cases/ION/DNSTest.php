<?php

namespace ION;


use ION\DNS;
use ION\RuntimeException;
use ION\Test\TestCase;

class DNSTest extends TestCase {

    public static $example_com = [
        'CNAME' => "example.com",
        'A'     => ["93.184.216.34"],
        'AAAA'  => ["2606:2800:220:1:248:1893:25c8:1946"]
    ];

    /**
     * @memcheck
     * @group dev
     */
    public function testResolve() {
        $this->promise(function () {
            $this->data['addr'] = yield DNS::resolve("example.com", DNS::RECORD_A | DNS::RECORD_AAAA  | DNS::RECORD_CNAME);
        });
        $this->loop();
        $this->assertEquals(self::$example_com, $this->data['addr']);
    }

    /**
     * @memcheck
     */
    public function testResolveFiled() {
        $this->promise(function () {
            $this->data['addr'] = yield DNS::resolve("unexist.example.com", DNS::RECORD_A | DNS::RECORD_AAAA  | DNS::RECORD_CNAME);
        });
        $this->loop();
        $this->assertEquals(
            $this->describe(new RuntimeException("DNS request failed: nodename nor servname provided, or not known", 0)),
            $this->data['error']
        );
    }
}