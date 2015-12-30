<?php

namespace ION\Stream;


use ION\Listener;

class Server {

	public function listen(string $host, int $backlog = -1) : Listener {}

	public function shutdownListener(string $host) : self {}

	public function enable() : self {}

	public function disable() : self {}
}