<?php

namespace ION\Process\IPC;

use ION\Process\ChildProcess;

/**
 * Сообщение межпроцессорного коммуникатора.
 *
 * @package ION\Process\IPC
 */
class DataMessage {
    const DATA_BINARY = 1;

    /**
     * @var int
     */
    public $type = self::DATA_BINARY;
    /**
     * @var mixed
     */
    public $context;
    /**
     * @var ChildProcess
     */
    public $process;
    /**
     * @var mixed
     */
    public $data;

}