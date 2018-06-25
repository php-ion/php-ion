<?php
/**
 *
 */

namespace ION;


class DescriptorEvent extends EventAbstract
{
    /**
     * Wait for a socket or FD to become readable.
     */
    const READ_EVENT = 4;
    /**
     * Wait for a socket or FD to become writeable.
     */
    const WRITE_EVENT = 8;
    /**
     * DescriptorEvent constructor.
     * @param resource|\SplFileObject|int|Stream|Listener $fd
     * @param int $flags
     */
    public function __construct($fd, int $flags = 0) { }
}