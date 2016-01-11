<?php

namespace ION\HTTP\Content;


use ION\Deferred;
use ION\Stream;

class Chunked {

    public function __construct(Stream $connect) { }

    public function readChunk(int $max_size = 0) : Deferred { }

    public function writeChunk(string $data) : self { }

}