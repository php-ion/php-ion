<?php
namespace ION\Test;

use ION;
use ION\Test\TestCase\Server;

class TestCase extends \PHPUnit_Framework_TestCase {

    const SERVER_CHUNK_INTERVAL = 5e4;
    const LOOP_TIMEOUT = 0.5;
    const WORKER_DELAY = -0.03;

    private $_stop;
    private $_error;
    public $data = [];
    public $shared;

    public function setUp() {
        $this->data = [];
    }

    public function getVarDir() {
        return dirname(__DIR__)."/var";
    }

    public function tearDown() {
        $doc = \PHPUnit_Util_Test::parseTestMethodAnnotations(get_class($this), $this->getName(false));
        if (isset($doc['method']['memcheck'])) {
            $memory = 0; // allocate variable
            $zero = 0; // allocate zero value
            $r = array(0, 0, 0);  // allocate result

            for ($i = 0; $i < 3; $i++) {
                $memory = memory_get_usage(0);
                $this->data = [];
                $this->runTest();
                unset($this->data);
//                ION::dispatch(ION::LOOP_NONBLOCK); // ammm, i think libevent free unpinned data-chunks after all
                $r[$i] = memory_get_usage(0) - $memory;
                if ($r[$i] < 0) { // free memory o_O
                    $r[$i] = $zero; // this hack works - no one 8-bytes-memory-leak
                }
            }

            if (array_sum($r)) {
                $this->fail("Memory leak detected: +" . implode(" B, +", $r) . " B");
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

    /**
     * Describe object or something else
     * @param mixed $data
     * @return mixed
     */
    public function describe($data) {
        if(is_object($data)) {
            if ($data instanceof \Throwable) {
                return [
                    'exception' => get_class($data),
                    'message' => $data->getMessage(),
                    'code' => $data->getCode()
                ];
            } else {
                $result = [
                    'object' => get_class($data),
                ];

                if (method_exists($data, "__toString")) {
                    $result['name'] = strval($data);
                }
                return $result;
            }
        } elseif(is_resource($data)) {
            return [
                "resource" => intval(STDIN)
            ];
        } else {
            return $data;
        }
    }

    public function exception2array($exception) {
        if($exception instanceof \Throwable) {
            return [
                'exception' => get_class($exception),
                'message' => $exception->getMessage(),
                'code'    => $exception->getCode()
            ];
        } else {
            return null;
        }
    }

    public function assertException($exception, $message = null, $code = null) {
        $this->assertInstanceOf('Exception', $exception);
        /* @var \Exception $exception */
        if (null !== $message) {
            $this->assertSame($message, $exception->getMessage());
        }
        if (null !== $code) {
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
        $wpid = pcntl_waitpid($pid, $s);
        if($wpid != -1) { // in gdb we lost sigchld
            $this->assertSame($pid, $wpid);
            $this->assertSame($status, $s);
        }
    }

    public function kill($pid) {
        posix_kill($pid, SIGTERM);
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
        if ($this->_stop) {
            if ($this->_error) {
                $this->fail($this->_error);
            }
        } elseif ($timeout_as_fail) {
            $this->fail("Loop timed out. " . ($this->data ? "Data: " . var_export($this->data, true) : ""));
        }
    }

    public function stop($error = null) {
        $this->_stop = true;
        if (is_numeric($error)) {
            ION::stop((double)$error);
        } else {
            $this->_error = $error;
            ION::stop(0.1);
        }

    }

    public function startWorker(callable $callback, $wait = self::WORKER_DELAY) {
        $pid = pcntl_fork();
        if ($pid == -1) {
            $this->fail("Fork failed");
        } elseif ($pid) {
            if ($wait < 0) {
                usleep(abs($wait) * 1e6);
            }
            return $pid;
        } else {
            if ($wait > 0) {
                usleep($wait * 1e6);
            }
            try {
                call_user_func($callback);
            } catch (\Exception $e) {
                error_log(strval($e));
                exit(127);
            }
            exit(0);
        }
    }

    public function out($message) {
        if (is_string($message)) {
            echo $message . "\n";
        } else {
            var_dump($message);
        }
        ob_flush();
    }

    public function dummy() {

    }
}
