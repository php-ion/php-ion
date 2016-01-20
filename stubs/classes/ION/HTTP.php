<?php

namespace ION;

use ION\HTTP\Request;
use ION\Stream;

class HTTP {

    public static function request(Request $request, Stream $socket, array $options = null) : Deferred { }

    public static function getResponseReason(int $response_code) : string { }
}