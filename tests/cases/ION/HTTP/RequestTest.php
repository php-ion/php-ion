<?php

namespace ION\HTTP;


use ION\Test\TestCase;

class RequestTest extends TestCase {

    /**
     */
    public function testParse() {
        $req = Request::parse(implode("\r\n", [
            "GET /iddqd?cheat=1 HTTP/1.1",
            "Host: example.com",
            "User-Agent: nobody",
            "Expires: -1",
            ""
        ]));
    }
}