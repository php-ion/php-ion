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
	public $timeout;

	public function __construct($host) {
		$this->host = $host;

	}

	public function onConnect(callable $cb) {
		$this->on_connect = $cb;
		return $this;
	}

	public function inWorker($timeout = TestCase::WORKER_WAIT_ON_START) {
		$this->timeout = $timeout;
		$this->in_worker = true;
		return $this;
	}

	public function start() {
		if(!$this->on_connect) {
			throw new \LogicException("No connection dispatcher");
		}
		if($this->in_worker) {
			$pid = pcntl_fork();
			if($pid == -1) {
				throw new \RuntimeException("Fork for server failed");
			} elseif($pid) {
				if($this->timeout < 0) {
					usleep(abs($this->timeout) * 1e6);
				}
				return $pid;
			} else {
				\ION::reinit();
				if($this->timeout > 0) {
					usleep($this->timeout * 1e6);
				}
				try {
					$this->_dispatch();
				} catch(\Exception $e) {
					error_log(strval($e));
					exit(127);
				}
				exit(0);
			}
		} else {
			$this->_dispatch();
		}
		return 0;
	}

	private function _dispatch() {

		$this->listen = stream_socket_server("tcp://{$this->host}", $errno, $error);
		if($errno) {
			throw new \RuntimeException("Failed to open server (tcp://{$this->host}): $error");
		}
		$connect = stream_socket_accept($this->listen);
		call_user_func($this->on_connect, $connect);
		fclose($connect);
		fclose($this->listen);
	}
}