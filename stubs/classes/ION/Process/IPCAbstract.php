<?php
/**
 * Created by PhpStorm.
 * User: bzick
 * Date: 15.08.16
 * Time: 22:35
 */

namespace ION\Process;


use ION\Promise;
use ION\Sequence;

abstract class IPCAbstract {

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
    public function whenDisconnect() : Promise {}

    /**
     * Возвращает время последнего сообщения
     *
     * @return float
     */
    public function getLastTime() : float  {}

    /**
     * Задает цепочку действий на входящее собщение.
     * Аргументом цепочки будет объект IPC\Message
     *
     * @return Sequence
     */
    public function whenMessage() : Sequence {}


    /**
     * Отправляет текстовые/бинарные данные
     *
     * @param string $data
     *
     * @return IPC
     */
    public function send(string $data) : self {}
}