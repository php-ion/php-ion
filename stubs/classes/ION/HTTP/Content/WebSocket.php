<?php

namespace ION\HTTP\Content;


use ION\Sequence;
use ION\Stream;

class WebSocket {

	public function __construct(Stream $connect) {}

	public function frame() : Sequence {}

	public function sendFrame() : self {}
}