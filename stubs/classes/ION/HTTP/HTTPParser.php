<?php
/**
 * Created by PhpStorm.
 * User: bzick
 * Date: 21.11.16
 * Time: 1:07
 */

namespace ION\HTTP;


use ION\Sequence;
use ION\Stream;

class HTTPParser extends Sequence {


    public function __construct() {
        parent::__construct(function (Stream $stream) {
            $headers = yield $stream->readLine("\r\n\r\n", Stream::MODE_TRIM_TOKEN, 64 * KiB);
            return Request::parse($headers);
        });
    }

    /**
     * @param Stream $source
     */
    public function __invoke(Stream $source) {
        parent::__invoke($source);
    }
}