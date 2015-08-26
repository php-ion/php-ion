<?php
namespace ION;

use ION;
use TestCase\Server;

class TestCase extends \PHPUnit_Framework_TestCase
{

	private $_stop;
	private $_error;
	public $data = [];
	public $shared;

	public function setUp() {
		$this->data = [];
	}

	public function tearDown() {
		$method = new \ReflectionMethod($this, $this->getName(false));

		if(strpos($method->getDocComment(), "@memcheck")) {
			$memory = 0; // allocate variable
			$zero = 0; // allocate zero value
			$r = array(0, 0);  // allocate result

			for($i=0; $i<2; $i++) {
				$memory = memory_get_usage();
				$this->runTest();
				$r[$i] = memory_get_usage() - $memory;
				if($r[$i] < 0) { // free memory o_O
					$r[$i] = $zero; // this hack works - no one 8-bytes-memory-leak
				}
			}

			if(array_sum($r)) {
				$this->fail("Memory leak detected: +".implode(" B, +", $r)." B");
			}
		}
	}

	/**
	 * @param string $host
	 * @return Server
	 */
	public function listen($host) {
		return new Server($host);
	}

	public function assertException($exception, $message = null, $code = null) {
		$this->assertInstanceOf('Exception', $exception);
		/* @var \Exception $exception */
		if(null !== $message) {
			$this->assertSame($message, $exception->getMessage());
		}
		if(null !== $code) {
			$this->assertSame($code, $exception->getCode());
		}
	}

	public function point($name, $time = 0.0) {

	}

	public function assertTimeout($time, $timeout, $accuracy = 0.01) {
		$time = microtime(1) - $time;
		$this->assertTrue($time >= $timeout, "Time great then timeout");
		$this->assertTrue($time < $timeout + $accuracy, "Value ($time) exceeds the accuracy ($timeout + $accuracy)");
	}


	public function assertWaitPID($pid, $status = 0) {
		$this->assertSame($pid, pcntl_waitpid($pid, $s));
		$this->assertSame($status, $s);
	}

	/**
	 * @param float $timeout
	 * @param bool $timeout_as_fail
	 */
	public function loop($timeout = 0.5, $timeout_as_fail = true) {
		$this->_stop = false;
		$this->_error = null;
//		$stopper = ION::await($timeout)->then(function ($tmp, $error) {
//			if(!$error) {
//				ION::stop();
//			}
//		});
		ION::dispatch();
//		$stopper->reject("already");
//		unset($stopper);
//		ION::reinit(ION::RECREATE_BASE);
		if($this->_stop) {
			if($this->_error) {
				$this->fail($this->_error);
			}
		} elseif($timeout_as_fail) {
			$this->fail("Loop timed out. ".($this->shared ? "Data: ".var_export($this->shared, true) : ""));
		}
	}

	public function stop($error = null) {
		$this->_stop = true;
		if(is_numeric($error)) {
			ION::stop((double)$error);
		} else {
			$this->_error = $error;
			ION::stop(0.1);
		}

	}

	public function startWorker(callable $callback, $wait = 0.1) {
		$pid = pcntl_fork();
		if($pid == -1) {
			$this->fail("Fork failed");
		} elseif($pid) {
			if($wait < 0) {
				usleep(abs($wait) * 1e6);
			}
			return $pid;
		} else {
			if($wait > 0) {
				usleep($wait * 1e6);
			}
			try {
				call_user_func($callback);
			} catch(\Exception $e) {
				error_log(strval($e));
				exit(127);
			}
			exit(0);
		}
	}

	public function out($message) {
		echo $message."\n";
		ob_flush();
	}
}
