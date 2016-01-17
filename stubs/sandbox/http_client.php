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

