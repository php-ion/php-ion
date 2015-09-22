<?php

namespace ION;


class DNS {

    const RECORD_A     = 1;
    const RECORD_AAAA  = 2;
    const RECORD_CNAME = 4;

    // todo
    const RECORD_NS    = 2;
    const RECORD_SOA   = 6;
    const RECORD_PTR   = 12;
    const RECORD_MX    = 15;
    const RECORD_TXT   = 16;

    /** Make a non-blocking getaddrinfo(3) request.
     * @param string $domain
     * @param int $flags
     * @return Deferred
     */
    public static function resolve($domain, $flags = self::RECORD_A | self::RECORD_AAAA) {}
}