<?php

namespace ION;

use ION\Test\TestCase;

class DNSTest extends TestCase {
    const UNEXISTS_DOMAIN = "unexist.example.com";

    public static $example_com = [
        'domain' => "example.com",
        'ipv4'   => ["93.184.216.34"],
        'ipv6'   => ["2606:2800:220:1:248:1893:25c8:1946"]
    ];

    /**
     * @group dev
     * @mem check
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
    public function testResolveFailed() {
        $this->promise(function () {
            $this->data['addr'] = yield DNS::resolve(self::UNEXISTS_DOMAIN, DNS::RECORD_A | DNS::RECORD_AAAA  | DNS::RECORD_CNAME);
        });
        $this->loop();
        $this->assertTrue(isset($this->data['error']));
        $this->assertEquals('ION\RuntimeException', $this->data['error']["exception"]);
    }
}