<?php

ION::promise(function () {
	$request = ION\HTTP\Request::factory([
		"method"  => "GET",
		"uri" => ION\URI::parse("http://example.com/"),
		"version" => "1.1",
		"headers" => [
			"user-agent" => "ION HTTP Client example"
		]
	]);
//	$request = (new \ION\HTTP\Request())
//		->withHeader("User-Agent", "ION HTTP Client example")
//		->withProtocolVersion("1.1")
//		->withUri(\ION\URI::parse("http://example.com/"));


	$response = yield \ION\HTTP::request($request);
	/* @var \ION\HTTP\Response $response */
	echo "Code: ".$response->getStatusCode() . "(" . $response->getReasonPhrase() . ")\n\n";
	echo "Headers: \n\n";
	var_dump($response->getHeaders());
	echo "Content: \n\n";
	var_dump(yield $response->readBody());
	echo "\n\n";
});


ION::dispatch();

