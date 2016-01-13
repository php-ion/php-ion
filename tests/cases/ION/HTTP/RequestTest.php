<?php

namespace ION\HTTP;


use ION\Test\TestCase;

class RequestTest extends TestCase {

    /**
     * @group dev
     * @memcheck
     */
    public function testParseHeaders() {
        $req = Request::parse(implode("\r\n", [
            "GET /iddqd?cheat=1 HTTP/1.1",
            "Host: example.com",
            "User-Agent: nobody",
            "Expires: -1",
            "Cookie: one=1",
            "Cookie: two=2",
            ""
        ]));

        $this->assertEquals("example.com", $req->getHeaderLine("Host"));
        $this->assertEquals("nobody", $req->getHeaderLine("User-Agent"));
        $this->assertEquals("-1", $req->getHeaderLine("Expires"));
        $this->assertEquals("one=1, two=2", $req->getHeaderLine("Cookie"));

        $this->assertEquals("/iddqd", $req->getUri()->getPath());
        $this->assertEquals("cheat=1", $req->getUri()->getQuery());

        $this->assertEquals("GET", $req->getMethod());
        $this->assertEquals("1.1", $req->getProtocolVersion());
    }
}