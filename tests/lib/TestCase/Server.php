<?php

namespace ION\Test\TestCase;


use ION\Test\TestCase;

class Server {
    /**
     * @var resource
     */
    public $listen;

    public $on_connect;
    public $in_worker;

    public function __construct($host) {
        $this->host = $host;
        $this->listen = stream_socket_server("tcp://{$this->host}", $errno, $error);
        if ($errno) {
            throw new \RuntimeException("Failed to open server (tcp://{$this->host}): $error");
        }
    }

    public function onConnect(callable $cb) {
        $this->on_connect = $cb;
        return $this;
    }

    public function inWorker() {
        $this->in_worker = true;
        return $this;
    }

    public function start() {
        var_dump("start"); ob_flush();
        if (!$this->on_connect) {
            throw new \LogicException("No connection dispatcher");
        }
        if ($this->in_worker) {
            $pid = pcntl_fork();
            if ($pid == -1) {
                throw new \RuntimeException("Fork for server failed");
            } elseif ($pid) {
                fclose($this->listen);
                unset($this->listen);
                return $pid;
            } else {
                \ION::reinit();
                try {
                    var_dump("dispatch"); ob_flush();
                    $this->_dispatch();
                } catch (\Exception $e) {
                    error_log(strval($e));
                    exit(127);
                }
                var_dump("done"); ob_flush();
                exit(0);
            }
        } else {
            $this->_dispatch();
        }
        return 0;
    }

    private function _dispatch() {
        $connect = stream_socket_accept($this->listen, 2);
        call_user_func($this->on_connect, $connect);
        fclose($connect);
        fclose($this->listen);
        unset($this->listen);
    }
}