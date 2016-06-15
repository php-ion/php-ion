<?php

namespace ION\HTTP;


use ION\Test\TestCase;
use ION\URI;

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

    /**
     * @memcheck
     */
    public function testParseBodyWithLength() {
        $body = str_pad("body:", 1000, ".");
        $req = Request::parse(implode("\r\n", [
            "PUT / HTTP/1.1",
            "Host: example.com",
            "Content-Length: ".strlen($body),
            "",
            $body
        ]));

        $this->assertEquals("example.com", $req->getHeaderLine("Host"));

        $this->assertEquals("/", $req->getUri()->getPath());

        $this->assertEquals("PUT", $req->getMethod());
        $this->assertEquals("1.1", $req->getProtocolVersion());
        $this->assertEquals($body, $req->getBody());
    }

    /**
     * @memcheck
     */
    public function testParseBodyChunked() {
        $chunk1 = str_pad("chunk1:", 11, ".");
        $chunk2 = str_pad("chunk2:", 12, ".");
        $chunk3 = str_pad("chunk3:", 13, ".");
        $req = Request::parse(implode("\r\n", [
            "PUT / HTTP/1.1",
            "Host: example.com",
            "Transfer-Encoding: chunked",
            "",
            dechex(strlen($chunk1)),
            $chunk1,
            dechex(strlen($chunk2)),
            $chunk2,
            dechex(strlen($chunk3)),
            $chunk3,
            "0",
            "",
        ]));

        $this->assertEquals("example.com", $req->getHeaderLine("Host"));

        $this->assertEquals("/", $req->getUri()->getPath());

        $this->assertEquals("PUT", $req->getMethod());
        $this->assertEquals("1.1", $req->getProtocolVersion());
        $this->assertEquals($chunk1.$chunk2.$chunk3, $req->getBody());
    }

    /**
     * @memcheck
     */
    public function testTarget() {
        $req = new Request();
        $this->assertEquals("", $req->getRequestTarget());
        $req->withRequestTarget("*");
        $this->assertEquals("*", $req->getRequestTarget());
    }

    /**
     * @memcheck
     */
    public function testToString() {
        $body = "...body content...";
        $request_str = implode("\r\n", [
            "GET /iddqd?cheat=1 HTTP/1.1",
            "Host: example.com",
            "User-agent: nobody",
            "Expires: -1",
            "Cookie: one=1",
            "cookie: two=2",
            "Content-Length: ".strlen($body),
            "",
            $body
        ]);
        $req = Request::parse($request_str);

        $expected = implode("\r\n", [
            "GET /iddqd?cheat=1 HTTP/1.1",
            "Host: example.com",
            "User-Agent: nobody",
            "Expires: -1",
            "Cookie: one=1,two=2",
            "Content-Length: ".strlen($body),
            "",
            $body
        ]);

        $this->assertEquals($expected, strval($req));
    }

    /**
     * @memcheck
     */
    public function testFactory() {
        $body = "...body content...";

        $req = Request::factory([
            Request::METHOD  => "GET",
            Request::URI     => URI::parse("/iddqd?cheat=1"),
            Request::VERSION => "1.1",
            Request::HEADERS => [
                "Host" => "example.com",
                "User-Agent" => ["nobody"],
                "Cookie" => [
                    "one=1",
                    "two=2"
                ]
            ],
            Request::BODY    => $body
        ]);

        $expected = implode("\r\n", [
            "GET /iddqd?cheat=1 HTTP/1.1",
            "Host: example.com",
            "User-Agent: nobody",
            "Cookie: one=1,two=2",
            "",
            $body
        ]);
        $this->assertEquals($expected, strval($req));
    }

}