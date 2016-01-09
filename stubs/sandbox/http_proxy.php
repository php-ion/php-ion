<?php

$client = new \ION\Stream\Storage();

$server = new \ION\Stream\Server();

$server->listen("0.0.0.0:8080");
$server->listen("0.0.0.0:8443")->encrypt(\ION\Crypto::server());

$server->incoming()->then(function (\ION\HTTP\Request $request_in) use ($client) {

	$uri = $request_in->getUri();
	$request_in->getBody(function(\ION\Stream $stream) {

	});
	$target_host = $uri->getHost().":".$uri->getPort();
	$stream = yield $client->fetchStream($target_host)->timeout(10);

	$request_out = $request_in->withUri($uri->relative());

	$response_out = yield \ION\HTTP::request($request_out, $stream);

	if(!$request_in->hasHeader("content-length")) {

	} else if($request_in->getHeaderLine("content-length") > 2 * MiB) {
		$reader = $request_in->getContentReader();
	} else {

	}

	if($request_in->isKeepAlive()) {

	}

//	$request_out
});


function($boundary, \ION\Stream $connect) {
	$reader = new MultiPartReader($connect, $boundary);
	while($part = yield $reader->readPart()) {
		$content = yield $part->readContent();
	}
}

class MultiPartReader {
	public function readPart() {
		return $this;
	}

	public function readContent() {
		return "";
	}
}

function(\ION\Stream $connect) {
	$reader = new ChunkedReader($connect);
	$data = yield $reader->readContent();
	while($part = yield $reader->readChunk()) {
		/* @var ChunkedReader $part */
		$content = yield $part->readContent();
	}
}

class ChunkedReader {
	public function readChunk() {
		return $this;
	}

	public function readContent() {
		return "";
	}
}

function(\ION\Stream $connect) {
	$reader = new WebSocketReader($connect);
	$reader->frame()->then(function(Frame $frame) {

	});
}

class WebSocketReader {
	public function frame() {
		return new \ION\Sequence();
	}
}

$request = new \ION\HTTP\Request();

