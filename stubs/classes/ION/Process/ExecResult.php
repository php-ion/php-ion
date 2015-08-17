<?php

namespace ION\Process;


class ExecResult {
	public $command  = "";
	public $pid      = 0;
	public $stdout   = "";
	public $stderr   = "";
	public $signaled = false;
	public $signal   = 0;
	public $code     = -1;
}