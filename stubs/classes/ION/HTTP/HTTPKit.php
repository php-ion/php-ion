<?php

namespace ION\HTTP;


class HTTPKit {

	public static function request(string $host) : Request {}
	public static function parseHeaders(string $headers) : Message {}
	public static function parseRequest(string $request) : Request {}
	public static function parseResponse(string $response) : Response {}
	public static function parseFrame(string $frame) : array {}
}