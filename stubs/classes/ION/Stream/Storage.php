<?php

namespace ION\Stream;


use ION\Deferred;
use ION\Stream;

class Storage extends StorageAbstract {
    const RELEASED = 1;
    const IN_USE   = 2;

    /**
     * @param Stream $socket
     * @param int $flags
     *
     * @return Storage
     */
    public function addStream(Stream $socket, int $flags = self::RELEASED) : self { }


    /**
     * @param string $stream_name
     *
     * @return Storage
     */
    public function removeStream(string $stream_name) : self { }

    /**
     * @param string $host
     *
     * @return Deferred
     */
    public function fetchStream(string $host) : Deferred { }

}