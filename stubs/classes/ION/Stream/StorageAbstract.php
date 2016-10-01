<?php


namespace ION\Stream;


use ION\Sequence;
use ION\Stream;

abstract class StorageAbstract {

    /**
     * Set maximum connections
     *
     * @param int $max
     *
     * @return StorageAbstract
     */
    public function setMaxPoolSize(int $max) : self { }

    /**
     * Set idle connection timeout.
     * After timeout timeout sequence will be invoked.
     *
     * @param int $sec
     *
     * @return StorageAbstract
     */
    public function setIdleTimeout(int $sec) : self { }

    public function setReadLimits(int $rate, int $burst) : self { }

    public function setWriteLimits(int $rate, int $burst) : self { }

    public function setPriority(int $priority) : self { }

    public function setInputSize(int $size) : self { }

    public function setPingInterval(int $ping_interval, int $ping_timeout) : self { }


    /**
     * @return Sequence
     */
    public function whenHandshake() : Sequence { }

    /**
     * @return Sequence
     */
    public function whenIncoming() : Sequence { }

    /**
     * @return Sequence
     */
    public function whenTimeout() : Sequence { }

    /**
     * @return Sequence
     */
    public function whenClose() : Sequence { }

    /**
     * @return Sequence
     */
    public function whenCorrupted() : Sequence { }

    /**
     *
     * @return Sequence
     */
    public function whenPing() : Sequence { }


    public function getStream(string $name) : Stream { }

    public function hasStream(string $name) : bool { }

    public function getStats() : array { }
}