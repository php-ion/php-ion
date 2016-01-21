<?php

namespace ION\HTTP;


use ION\Sequence;

class MultiPartParser extends Sequence {

    public function __construct(string $boundary, int $content_length) { }

    public function getBoundary() : string {}

    public function isFinished() : bool {}

    public function __invoke(string $data) { }
}