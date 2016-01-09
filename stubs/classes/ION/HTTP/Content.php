<?php

namespace ION\HTTP;


use ION\Deferred;
use ION\Stream;

class Content {

	public function __construct(Stream $connect, Message $message) {}

	public function readContent(int $max_size = 0) : Deferred {}

}