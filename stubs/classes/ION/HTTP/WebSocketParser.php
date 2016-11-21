<?php

namespace ION\HTTP;


use ION\Sequence;

class WebSocketParser extends Sequence {

    public function isFinished() : bool { }

    public function frame() : Sequence { }

    public function getParsedCount() : int { }

    public function __invoke($source) { }
}