<?php

use ION\HTTP\Request;

ION::promise(function () {
	$request = Request::factory([
		Request::METHOD  => "GET",
        Request::URI     => ION\URI::parse("http://example.com/"),
        Request::VERSION => "1.1",
        Request::HEADERS => [
			"user-agent" => "ION HTTP Client example"
		]
	]);


    $parser = new \ION\HTTP\WebSocketParser;
    $request->onBody($parser)->then(function (\ION\HTTP\WebSocket\Frame $frame) {

    });
    while(!$parser->isFinished()) {
        $frame = yield $request->onBody($parser);
    }
    $parser = new \ION\HTTP\MultiPartParser("iddqd", 1000);
    $request->onBody($parser)->then(function (\ION\HTTP\MultiPart\Part $part) {

    });
    while(!$parser->isFinished()) {
        $part = yield $request->onBody($parser);
    }
    $parser = new \ION\HTTP\ChunkedParser;
    $request->onBody($parser)->then(function (string $chunk) {

    });
    while(!$parser->isFinished()) {
        $chunk = yield $request->onBody($parser);
    }

	$response = yield \ION\HTTP::request($request);
	/* @var \ION\HTTP\Response $response */
	echo "Code: ".$response->getStatusCode() . " (" . $response->getReasonPhrase() . ")\n\n";
	echo "Headers: \n\n";
	var_dump($response->getHeaders());
	echo "Content: \n\n";
	var_dump($response->getBody());
	echo "\n\n";
});


ION::dispatch();

