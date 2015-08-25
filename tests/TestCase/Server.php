<?php

namespace TestCase;


class Server {
	/**
	 * @var resource
	 */
	public $listen;

	public $on_connect;

	public function __construct($host) {
		$this->listen = stream_socket_server("tcp://{$host}", $errno, $error);
		if($errno) {
			throw new \RuntimeException("Failed to open server (tcp://{$host}): $error");
		}
	}

	public function onConnect(callable $cb) {
		$this->on_connect = $cb;
		return $this;
	}

	public function start() {
		if($this->on_connect) {
			$connect = stream_socket_accept($this->listen);
			call_user_func($this->on_connect, $connect);
			fclose($connect);
		}
	}
}