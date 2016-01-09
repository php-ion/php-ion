<?php

use ION\Listener;
use ION\Stream;

$server = new Listener("0.0.0.0:8080");

$server->onConnect(function (Stream $stream) {
	$head = (yield $stream->awaitLine("\r\n", Stream::MODE_TRIM_TOKEN, 64 * KiB));
	// ... parse $head
	$headers = (yield $stream->awaitLine("\r\n\r\n", Stream::MODE_TRIM_TOKEN, 64 * KiB));
	// ... parse $head
	$dest_host = $head;
	$dest = Stream::socket($dest_host);
	(yield $dest->write($head."\r\n".$headers)->flush());
	$head = (yield $dest->awaitLine("\r\n", Stream::MODE_TRIM_TOKEN, 64 * KiB));
	// ... parse $head
	$headers = (yield $dest->awaitLine("\r\n\r\n", Stream::MODE_TRIM_TOKEN, 64 * KiB));
	// ... parse $head
	$stream->write($head."\r\n".$headers);
});