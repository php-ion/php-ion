<?php

use ION\HTTP\Request;

ION::promise(function () {
	$request = Request::factory([
		Request::METHOD  => "GET",
        Request::URI     => ION\URI::parse("http://example.com/"),
        Request::VERSION => "1.1",
        Request::HEADERS => [
			"User-Agent" => "ION HTTP Client example",
            "X-Client"   => "PHP"
		]
	]);

    $respose = \ION\HTTP::request($request, \ION\Stream::socket("example.com:80"));

    $parser = new \ION\HTTP\WebSocketParser;
    if($frame = $parser($chunk)) {

    }
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


$init = function ($request) {

    throw \ION\Sequence::quit();
};


ION::dispatch();

