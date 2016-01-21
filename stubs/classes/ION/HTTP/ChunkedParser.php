<?php

namespace ION\HTTP;


use ION\Sequence;

class ChunkedParser extends Sequence {

    public function isFinished() : bool { }

    public function __invoke(string $data) { }
}