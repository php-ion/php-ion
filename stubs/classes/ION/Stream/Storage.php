<?php

namespace ION\Stream;


use ION\Stream;

class Storage extends StorageAbstract {
	const RELEASED = 1;
	const IN_USE   = 2;

	/**
	 * @param Stream $socket
	 * @param int $flags
	 * @return Storage
	 */
	public function addStream(Stream $socket, int $flags = self::RELEASED) : self {}


	/**
	 * @param string $name
	 * @return Storage
	 */
	public function removeStream(string $name) : self {}
}