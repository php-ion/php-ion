<?php

namespace ION;


class DNS {

    const ADDR_IPV4 = 1;
    const ADDR_IPV6 = 2;
    const ADDR_IP_ANY = 2;

    /** Make a non-blocking getaddrinfo request.
     * @param string $domain
     * @param int $flags
     */
    public static function getAddrInfo($domain, $flags = self::ADDR_IP_ANY) {}
}