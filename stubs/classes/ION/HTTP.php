<?php

namespace ION;

use ION\HTTP\Request;
use ION\HTTP\Response;
use ION\Stream;

class HTTP {

	public static function request(Request $request, Stream $socket = null) : Deferred {}
	public static function parseRequest(string $request) : Request {}
	public static function parseResponse(string $response) : Response {}
	public static function parseWSFrame(string $frame) : array {}
}