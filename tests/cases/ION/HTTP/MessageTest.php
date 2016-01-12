<?php

namespace ION\HTTP;


use ION\Test\TestCase;

class MessageTest extends TestCase {

    /**
     * @memcheck
     */
    public function testProtocolVersion() {
        $message = new Message();
        $this->assertEquals("1.1", $message->getProtocolVersion());
        $message->withProtocolVersion("2.0");
        $this->assertEquals("2.0", $message->getProtocolVersion());
    }

    /**
     * @memcheck
     */
    public function testHas() {
        $message = new Message();
        $this->assertFalse($message->hasHeader("Agent"));
        $this->assertFalse($message->hasHeader(""));
    }

    /**
     * @memcheck
     */
    public function testWithOne() {
        $message = new Message();
        $message->withAddedHeader("X-Header-Scalar", 22);
        $message->withAddedHeader("X-Header-Scalar", 22.7);
        $message->withAddedHeader("X-Header-Scalar", true);
        $message->withAddedHeader("X-Header-Scalar", "str");
        $message->withHeader("User-Agent", "unit test 1");
        $message->withHeader("User-Agent", "unit test 2");
        $message->withAddedHeader("User-agent", "unit test 3");

        $this->assertTrue($message->hasHeader("X-Header-Scalar"));
        $this->assertTrue($message->hasHeader("User-Agent"));
        $this->assertTrue($message->hasHeader("useR-Agent"));
        $this->assertFalse($message->hasHeader("Agent"));

        $this->assertEquals([
            "x-header-scalar" => ["22", "22.7", "1", "str"],
            "user-agent" => ["unit test 2", "unit test 3"]
        ], $message->getHeaders());
    }

    /**
     * @memcheck
     */
    public function testWithMulti() {
        $message = new Message();
        $message->withAddedHeader("X-Header-Scalar", [22, 22.7, true, "str"]);
        $message->withHeader("User-Agent", ["unit test 1"]);
        $message->withHeader("User-Agent", ["unit test 2", "unit test 3"]);
        $message->withAddedHeader("User-agent", "unit test 4");

        $this->assertTrue($message->hasHeader("X-Header-Scalar"));
        $this->assertTrue($message->hasHeader("User-Agent"));
        $this->assertTrue($message->hasHeader("useR-Agent"));
        $this->assertFalse($message->hasHeader("Agent"));

        $this->assertEquals([
            "x-header-scalar" => ["22", "22.7", "1", "str"],
            "user-agent" => ["unit test 2", "unit test 3", "unit test 4"]
        ], $message->getHeaders());
    }

    /**
     * @memcheck
     */
    public function testGet() {
        $message = new Message();
        $message->withHeader("User-Agent", ["unit test 2", "unit test 3"]);
        $message->withHeader("X-Tra", "lookup");

        $this->assertEquals(["unit test 2", "unit test 3"], $message->getHeader("User-Agent"));
        $this->assertEquals(["lookup"], $message->getHeader("X-Tra"));

        $this->assertEquals("unit test 2,unit test 3", $message->getHeaderLine("User-Agent"));
        $this->assertEquals("lookup", $message->getHeaderLine("X-Tra"));
    }

    /**
     * @memcheck
     */
    public function testWithout() {
        $message = new Message();
        $message->withHeader("User-Agent", ["unit test 2", "unit test 3"]);
        $message->withHeader("X-Tra", "lookup");
        $message->withHeader("X", "xxx");

        $this->assertTrue($message->hasHeader("User-Agent"));
        $this->assertTrue($message->hasHeader("X-Tra"));
        $this->assertTrue($message->hasHeader("X"));

        $message->withoutHeader("X-Tra");
        $message->withoutHeader("X");

        $this->assertTrue($message->hasHeader("User-Agent"));
        $this->assertFalse($message->hasHeader("X-Tra"));
        $this->assertFalse($message->hasHeader("X"));
    }
}