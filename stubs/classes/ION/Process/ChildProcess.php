<?php

namespace ION\Process;


use ION\Promise;

/**
 * Объект дочернего процесса.
 * Позволяет создать дочерний процесс, наладить с ним коммуникацию и узнать когда процесс завершится.
 *
 * @package ION\Process
 *
 * @source src/classes/ION/Process/ChildProcess.c
 * @source src/ion/process.c
 * @source src/ion/process.h
 */
class ChildProcess {
    /**
     * Возвращает время запуска дочернего процесса
     * @return float
     */
    public function getStartedTime() : float {}

    /**
     * Gets current process ID
     * @return int
     */
    public function getPID() : int {}

    /**
     * Сообщает что процесс еще запущен
     * @return bool
     */
    public function isAlive() : bool {}

    /**
     * Сообщает был ли запущен процесс
     * @return bool
     */
    public function isStarted() : bool {}

    /**
     * Сообщает был ли завершен процесс
     * @return bool
     */
    public function isFinished() : bool {}

    /**
     * Сообщает был ли завершен процесс сигналом
     * @return bool
     */
    public function isSignaled() : bool {}

    /**
     * Если процесс был завершен сигналом то вернет номер сигнала, иначе 0
     * @return int
     */
    public function getSignal() : int {}

    /**
     * Если процесс завершился сам то вернет код выхода процесса, иначе 0
     * @return int
     */
    public function getExitStatus() : int {}

    /**
     * Обещание на завершение дочернего процесса.
     * Обещаение выполняется в родительском процессе. В обещание передается сам объект дочернего процесса
     * @return Promise
     */
    public function whenExit() : Promise {}

    /**
     * Обещание на запуск дочернего процесса.
     * Обещаение выполняется в родительском процессе. В обещание передается сам объект дочернего процесса
     *
     * @return Promise
     */
    public function whenStarted() : Promise {}

    /**
     * Возвращает готовый IPC канал
     * @return IPC
     */
    public function getIPC() : IPC {}

    /**
     * Отложенно запускает дочерний процесс.
     * Дочерний процесс будет создан после возвращение интерпретатора в диспетчер, то есть не сразу.
     * Это позволяет работать без лишних условий что дочерний процесс уже существует.
     * Коллбек $callback будет вызван в дочернем процессе непременно. Обещание будет исполнено в родительском процессе
     *
     * @param callable $callback
     * @param int $flags
     *
     * @return ChildProcess
     */
    public function start(callable $callback, int $flags = 0) : self {}

}