<?php

namespace ION\Stream;


use ION\Server;
use ION\Stream;

class Connection extends Stream {

    public function getServer() : Server {}

    public function free() : self {}

    public function isFree() : bool {}
}