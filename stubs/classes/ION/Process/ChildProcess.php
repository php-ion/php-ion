<?php

namespace ION\Process;


use ION\Promise;

class ChildProcess  {
    public function getStartedTime() : float {}

    /**
     * Gets current process ID
     * @return int
     */
    public function getPID() : int {}

    public function isAlive() : bool {}
    public function isStarted() : bool {}
    public function isFinished() : bool {}

    public function isSignaled() : bool {}

    public function getSignal() : int {}

    public function getExitStatus() : int {}

    public function released() : Promise {}

    /**
     * Отложенно запускает дочерний процесс.
     * Дочерний процесс будет создан после возвращение интерпретатора в диспетчер, то есть не сразу.
     * Это позволяет работать без лишних условий что дочерний процесс уже существует.
     * Коллбек $callback будет вызван в дочернем процессе непременно.
     *
     * @param callable $callback calls in child process after spawn
     * @param callable $after call in parent process after spawn
     *
     * @return Promise
     */
    public function run() : Promise {}

    public function ipc() : IPC {}

}