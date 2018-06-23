<?php
/**
 *
 */

namespace ION;


class DescriptorEvent extends EventAbstract
{
    const READ_EVENT = 4;
    const WRITE_EVENT = 8;
    /**
     * DescriptorEvent constructor.
     * @param resource|\SplFileObject|int $fd
     * @param int $flags
     */
    public function __construct($fd, int $flags = 0) { }
}