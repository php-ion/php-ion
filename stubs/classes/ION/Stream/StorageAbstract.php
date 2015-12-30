<?php


namespace ION\Stream;


use ION\Sequence;
use ION\Stream;

abstract class StorageAbstract {

	/**
	 * Set maximum connections
	 * @param int $max
	 * @return StorageAbstract
	 */
	public function setMaxPoolSize(int $max) : self {}

	/**
	 * Set idle connection timout.
	 * After timeout timeout sequence will be invoked.
	 * @param int $sec
	 * @return Server
	 */
	public function setIdleTimeout(int $sec) : self {}

	public function setReadLimits(int $rate, int $burst) : self {}

	public function setWriteLimits(int $rate, int $burst) : self {}

	public function setPriority(int $priority) : self {}

	public function setInputSize(int $size) : self {}

	public function setPingInterval(int $ping_interval, int $ping_timeout) : self {}


	public function handshake() : Sequence {}

	public function incoming() : Sequence {}

	public function timeout() : Sequence {}

	public function close() : Sequence {}

	public function ping() : Sequence {}


	public function getStream(string $name) : Stream {}

	public function hasStream(string $name) : bool {}

	public function getStats() : array {}
}