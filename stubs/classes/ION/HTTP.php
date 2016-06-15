<?php

namespace ION;

use ION\HTTP\Request;
use ION\HTTP\Response;
use ION\Stream;

class HTTP {

    /**
     * @param Request $request
     * @param \ION\Stream $socket
     *
     * @return Deferred
     */
    public static function request(Request $request, Stream $socket) : Deferred {
        return Response::parse(yield $socket->write($request->build())->readLine("\r\n\r\n", Stream::MODE_TRIM_TOKEN, 8192)->timeout(30));
    }

    public static function getResponseReason(int $response_code) : string { }
}