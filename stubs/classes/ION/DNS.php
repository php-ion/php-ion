<?php

namespace ION;


class DNS {

    const ADDR_IPV4 = 1;
    const ADDR_IPV6 = 2;
    const ADDR_IP_ANY = 2;

    const RECORD_TYPE_A     = 1;
    const RECORD_TYPE_AAAA  = 28;
    const RECORD_TYPE_CNAME = 5;
    const RECORD_TYPE_MX    = 15;
    const RECORD_TYPE_NS    = 2;
    const RECORD_TYPE_PTR   = 12;
    const RECORD_TYPE_SOA   = 6;
    const RECORD_TYPE_TXT   = 16;

    /** Make a non-blocking getaddrinfo request.
     * @param string $domain
     * @param int $flags
     * @return Deferred
     */
    public static function getAddrInfo($domain, $flags = self::ADDR_IP_ANY) {}

    /**
     * @param $domain
     * @param int $types
     * @return Deferred
     */
    public static function resolve($domain, $types = self::RECORD_TYPE_A | self::RECORD_TYPE_AAAA) {}

    public static function resolveReverse($ip) {}
}