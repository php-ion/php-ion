<?php

namespace ION\Stream;


use ION\Crypto;
use ION\Deferred;

class Client extends StorageAbstract {

	/**
	 * @param string $target
	 * @param string $peer_name
	 * @param Crypto $ssl
	 */
	public function addTarget(string $target, string $peer_name, Crypto $ssl = null) {}

	public function fetchStream(string $target = null) : Deferred {}
}