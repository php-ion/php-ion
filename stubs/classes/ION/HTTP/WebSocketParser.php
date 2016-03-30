<?php

namespace ION\HTTP;


use ION\Sequence;

class WebSocketParser {

    public function isFinished() : bool { }

    public function frame() : Sequence { }

    public function getParsedCount() : int { }

    public function hasUnparsedFrame() : bool { }

    public function __invoke(string $raw) { }
}