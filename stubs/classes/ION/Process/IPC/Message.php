<?php

namespace ION\Process\IPC;

use ION\Process\ChildProcess;

/**
 * Сообщение межпроцессорного коммуникатора.
 *
 * @package ION\Process\IPC
 *
 */
class Message {
    /**
     * @var mixed|ChildProcess
     */
    public $context;
    /**
     * @var mixed
     */
    public $data;

}