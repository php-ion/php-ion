<?php

namespace ION\HTTP\Content\MultiParted;


use ION\Deferred;
use ION\HTTP\Message;
use ION\Stream;

class Part extends Message {

	public function __construct(Stream $connect) {}

	public function readContent(int $max_size = 0) : Deferred {}
}