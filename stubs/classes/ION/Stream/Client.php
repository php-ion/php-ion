<?php

namespace ION\Stream;


use ION\Crypto;
use ION\Deferred;

class Client extends StorageAbstract {

	public function addTarget(string $target, string $host, Crypto $ssl = null) {}

	public function fetchStream(string $target = null) : Deferred {}
}