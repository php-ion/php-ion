<?php

$client = new \ION\Stream\Storage();

$server = new \ION\Stream\Server();

$server->listen("0.0.0.0:8080");
$server->listen("0.0.0.0:8443")->encrypt(\ION\Crypto::server());

$server->incoming()->then(function (\ION\HTTP\Request $proxy_req) use ($client) {

	$uri = $proxy_req->getUri();
    $body = new \ION\HTTP\Body($proxy_req);
    $body = new \ION\HTTP\Body\MultiParted($proxy_req);
    while($part = yield $body->readPart(2 * MiB)) {
        if($part->isComplete()) {

        } else {
            while($chunk = yield $part->read(2 * MiB)) {

            }
        }
    }
	$target_host = $uri->getHost().":".$uri->getPort();
	$stream = yield $client->fetchStream($target_host)->timeout(10);

	$request = $proxy_req->withUri($uri->relative());

//	$response_out = yield $stream->write($request)->flush();
//
//	if(!$proxy_req->getHeaderLine("transfer-encoding") == "Chunked") {
//		$proxy_content = new \ION\HTTP\Content\Chunked($stream);
//		$content = new \ION\HTTP\Content\Chunked($stream);
//	} else if($proxy_req->getHeaderLine("content-length") > 2 * MiB) {
//		$reader = $proxy_req->getContentReader();
//	} else {
//
//	}
//
//	if($proxy_req->isKeepAlive()) {
//
//	}

});

//
//function($boundary, \ION\Stream $connect) {
//	$reader = new MultiPartReader($connect, $boundary);
//	while($part = yield $reader->readPart()) {
//		$content = yield $part->readContent();
//	}
//}
//
//class MultiPartReader {
//	public function readPart() {
//		return $this;
//	}
//
//	public function readContent() {
//		return "";
//	}
//}
//
//function(\ION\Stream $connect) {
//	$reader = new ChunkedReader($connect);
//	$data = yield $reader->readContent();
//	while($part = yield $reader->readChunk()) {
//		/* @var ChunkedReader $part */
//		$content = yield $part->readContent();
//	}
//}
//
//class ChunkedReader {
//	public function readChunk() {
//		return $this;
//	}
//
//	public function readContent() {
//		return "";
//	}
//}
//
//function(\ION\Stream $connect) {
//	$reader = new WebSocketReader($connect);
//	$reader->frame()->then(function(Frame $frame) {
//
//	});
//}
//
//class WebSocketReader {
//	public function frame() {
//		return new \ION\Sequence();
//	}
//}
//
//$request = new \ION\HTTP\Request();

