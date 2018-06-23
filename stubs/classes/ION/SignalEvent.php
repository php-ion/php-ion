<?php
/**
 *
 */

namespace ION;


class SignalEvent extends EventAbstract
{
    /**
     * SignalEvent constructor.
     * @param int $signal
     * @param int $flags
     *
     * @see Process::SIG
     */
    public function __construct($signal, int $flags = 0) { }
}