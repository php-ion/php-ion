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
class IPC {

    /**
     * @param mixed $ctx1 context for IPC channel one
     * @param mixed $ctx2 context for IPC channel two
     *
     * @return array|IPC[]
     */
    public static function create($ctx1 = null, $ctx2 = null) : array {}

    /**
     * @return mixed
     */
    public function getContext() {}

    /**
     * Сообщает что соединение установлено
     *
     * @return bool
     */
    public function isConnected() : bool {}

    /**
     * Дает обещание уведомить на разрыв соединения
     *
     * @return Promise
     */
    public function whenDisconnected() : Promise {}

    /**
     * Возвращает время последнего сообщения
     *
     * @return float
     */
//    public function getLastTime() : float  {}

    /**
     * Задает цепочку действий на входящее собщение.
     * Аргументом цепочки будет объект IPC\Message
     *
     * @return Sequence
     */
    public function whenIncoming() : Sequence {}


    /**
     * Отправляет текстовые/бинарные данные
     *
     * @param string $data
     *
     * @return IPC
     */
    public function send(string $data) : self {}
}