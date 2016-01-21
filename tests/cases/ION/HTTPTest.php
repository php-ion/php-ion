<?php

namespace ION;


use ION\HTTP\Content\MultiParted;
use ION\HTTP\Content\WebSocket\Frame;
use ION\HTTP\Request;
use ION\HTTP\Response;
use ION\Test\TestCase;

class HTTPTest extends TestCase {

    public function testRequest() {

        $this->promise(function () {

            $request = Request::factory([
                Request::METHOD  => "GET",
                Request::VERSION => "1.1",
                Request::URI     => URI::factory([
                    URI::PATH => "/"
                ])
            ]);


            $resp = yield HTTP::request($request, Stream::socket("example.com:80"));
            /** @var Response $resp */
            if($resp->getStatusCode() == 101) { // websocket
                $ws_out = new HTTP\Content\WebSocket($request);
                $ws_out->send(Frame::factory("hello"));
            }
        });

        $this->promise(function () {
            $request = Request::factory([
                Request::METHOD  => "GET",
                Request::VERSION => "1.1",
                Request::URI     => URI::factory([
                    URI::PATH   => "/"
                ])
            ]);


            $resp = yield $request->send(Stream::socket("example.com:443", Crypto::client()));
            /** @var Response $resp */
        });
    }
}