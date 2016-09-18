<?php

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

/**
 * Межпроцессорный коммуникатор.
 * Отрывает двунаправлыеный поток для передачи данных.
 *
 * @package ION\Process
 *
 * @source src/classes/ION/Process/IPC.c
 * @source src/ion/process.c
 * @source src/ion/process.h
 */
class IPC extends IPCAbstract {

    /**
     * @param mixed $ctx1 context for IPC channel one
     * @param mixed $ctx2 context for IPC channel two
     *
     * @return array|IPC[]
     */
    public static function create($ctx1 = null, $ctx2 = null) : array {}

    public function getContext() {}
}