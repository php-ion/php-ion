<?php

namespace cases\ION;


use ION\DNS;
use ION\Test\TestCase;

class _DNSTest_ extends TestCase {

    public static $example_com = [
        'CNAME' => "example.com",
        'A'     => ["93.184.216.34"],
        'AAAA'  => ["2606:2800:220:1:248:1893:25c8:1946"]
    ];

    /**
     * @memcheck
     */
    public function testGetAddrInfo() {
        DNS::resolve("example.com", DNS::RECORD_A | DNS::RECORD_AAAA  | DNS::RECORD_CNAME)->then(function($addrs) {
            $this->data['addr'] = $addrs;
            $this->stop();
        });
        $this->loop();
        $this->assertSame(self::$example_com, $this->data['addr']);
        unset($this->data);
    }
}