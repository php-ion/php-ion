<?php
namespace ION;

class TestCase extends \PHPUnit_Framework_TestCase
{

	public function setUp() {

	}

	public function tearDown() {
		$method = new \ReflectionMethod($this, $this->getName(false));

		if(strpos($method->getDocComment(), "@memcheck")) {
			$memory = 0; // reserve variable
			$r = array(0, 0);

			for($i=0; $i<2; $i++) {
				$memory = memory_get_usage();
				$this->runTest();
				$r[$i] = memory_get_usage() - $memory;
			}

			if(array_sum($r)) {
				$this->fail("Memory leak detected: +".implode(" B, +", $r)." B");
			}
		}
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

	public function separate(callable $callback, $wait = 0.1) {
		$pid = pcntl_fork();
		if($pid == -1) {
			$this->fail("Fork failed");
		} elseif($pid) {
			return $pid;
		} else {
			usleep($wait * 1e6);
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
