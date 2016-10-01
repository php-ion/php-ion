<?php

namespace ION\Stream;


use ION\Crypto;
use ION\Deferred;

class Client extends StorageAbstract {

    /**
     * @param string $target
     * @param string $host
     * @param Crypto $ssl
     *
     * @return Client
     */
    public function addTarget(string $target, string $host, Crypto $ssl = null) : self { }

    public function fetchStream(string $target = null) : Deferred { }
}