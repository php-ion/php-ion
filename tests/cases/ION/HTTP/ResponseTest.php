<?php

namespace ION\HTTP;


use ION\Test\TestCase;

class ResponseTest extends TestCase {

    /**
     * @memcheck
     */
    public function testInstance() {
        $resp = new Response();
    }

    /**
     * @memcheck
     */
    public function testParse() {
        $resp = Response::parse(implode("\r\n", [
            "HTTP/1.1 404 Not Exists",
            "Server: ion",
            "Expires: -1",
            "Cookie: one=1",
            "Cookie: two=2",
            ""
        ]));

        $this->assertEquals("ion", $resp->getHeaderLine("Server"));
        $this->assertEquals("-1", $resp->getHeaderLine("Expires"));
        $this->assertEquals("one=1, two=2", $resp->getHeaderLine("Cookie"));

        $this->assertEquals(404, $resp->getStatusCode());
        $this->assertEquals("Not Exists", $resp->getReasonPhrase());
        $this->assertEquals("1.1", $resp->getProtocolVersion());
    }


    /**
     * @memcheck
     */
    public function testParseBodyWithLength() {
        $body = str_pad("body:", 1000, ".");
        $resp = Response::parse(implode("\r\n", [
            "HTTP/1.1 200 OK",
            "Server: ion",
            "Content-Length: ".strlen($body),
            "",
            $body
        ]));

        $this->assertEquals("ion", $resp->getHeaderLine("Server"));

        $this->assertEquals(200, $resp->getStatusCode());
        $this->assertEquals("OK", $resp->getReasonPhrase());
        $this->assertEquals("1.1", $resp->getProtocolVersion());
        $this->assertEquals($body, $resp->getBody());
    }

    /**
     * @memcheck
     */
    public function testParseBodyChunked() {
        $chunk1 = str_pad("chunk1:", 11, ".");
        $chunk2 = str_pad("chunk2:", 12, ".");
        $chunk3 = str_pad("chunk3:", 13, ".");
        $resp = Response::parse(implode("\r\n", [
            "HTTP/1.1 200 OK",
            "Server: ion",
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

        $this->assertEquals("ion", $resp->getHeaderLine("Server"));

        $this->assertEquals(200, $resp->getStatusCode());
        $this->assertEquals("OK", $resp->getReasonPhrase());
        $this->assertEquals("1.1", $resp->getProtocolVersion());
        $this->assertEquals($chunk1.$chunk2.$chunk3, $resp->getBody());
    }

    /**
     * @memcheck
     */
    public function testToString() {
        $body = "...body content...";
        $request_str = implode("\r\n", [
            "HTTP/1.1 100 Continue",
            "Server: ion",
            "Expires: -1",
            "Cookie: one=1",
            "Cookie: two=2",
            "Content-Length: ".strlen($body),
            "",
            $body
        ]);
        $resp = Response::parse($request_str);

        $expected = implode("\r\n", [
            "HTTP/1.1 100 Continue",
            "Server: ion",
            "Expires: -1",
            "Cookie: one=1,two=2",
            "Content-Length: ".strlen($body),
            "",
            $body
        ]);

        $this->assertEquals($expected, strval($resp));
    }
}