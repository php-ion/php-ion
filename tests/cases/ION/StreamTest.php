<?php

namespace ION;

use ION;
use ION\Stream\ConnectionException;
use ION\Test\TestCase;

class StreamTest extends TestCase {

    public function setupSendServer($data) {
        return $this->listen(ION_TEST_SERVER_HOST)->inWorker()->onConnect(function ($connect) use ($data) {
//            $this->out("connected");
            if($data) {
                if (is_array($data)) {
                    foreach ($data as $chunk) {
                        fwrite($connect, $chunk);
                        usleep(self::SERVER_CHUNK_INTERVAL); // 0.1s
                    }
                } else {
                    fwrite($connect, $data);
                }
            } else {
                usleep(self::SERVER_CHUNK_INTERVAL);
            }
//            $this->out("closed");
        })->start();
    }

    public function setupStoreServer() {
        return $this->listen(ION_TEST_SERVER_HOST)->inWorker()->onConnect(function ($connect) {
//            $this->out("open");
            $file = fopen($this->getVarDir().'/server.data', "w");
            stream_copy_to_stream($connect, $file);
            fclose($file);
//            $this->out("close");
        })->start();
    }

    public function getServerResult() {
        $this->assertFileExists($this->getVarDir().'/server.data');
        $data =  file_get_contents($this->getVarDir().'/server.data');
        unlink($this->getVarDir().'/server.data');
        return $data;
    }

    /**
     * @memcheck
     */
    public function testCreatePair() {
        $pair = Stream::pair();
        $this->assertCount(2, $pair);
        $this->assertInstanceOf('ION\Stream', $pair[0]);
        $this->assertInstanceOf('ION\Stream', $pair[1]);
    }

    /**
     * @memcheck
     */
    public function testCreateFromResource() {
        $stream = Stream::resource(STDIN);
        $this->assertInstanceOf('ION\Stream', $stream);
    }

    /**
     * @memcheck
     */
    public function testSocket() {
        $listener = new Listener(ION_TEST_SERVER_IPV4);
        $stream = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
        $this->assertInstanceOf('ION\Stream', $stream);
    }

    /**
     * @memcheck
     */
    public function testEnableDisable() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $a->disable()->enable();
        $b->disable()->disable()->enable()->enable();
    }

    /**
     *
     * @memcheck
     */
    public function testAppendToInput() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $a->appendToInput("0123456789");
        $this->assertSame("0123456789", $a->getAll());
        $this->assertSame("", $b->getAll());
    }

    /**
     * @memcheck
     */
    public function testWrite() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $a->write("0123456789");
        $this->assertSame("0123456789", $b->getAll());
        $this->assertSame("", $a->getAll());
    }

    public function providerString() {
        $string = "0123456789";
        return array(
            [$string, "0", 0],
            [$string, "012", 0],
            [$string, "012", -1, 1],
            [$string, $string, 0],
            [$string, "4", 4],
            [$string, "45", 4],
            [$string, "45", 4, 2, 6],
            [$string, "45", -1, 5, 6],
            [$string, "45", -1, 2, 4],
            [$string, "9", 9],
            [$string, "89", 8],
            [$string, "89", 8, 6, 100],
            [$string, "89", -1, 9, 10],
        );
    }

    /**
     * @memcheck
     * @dataProvider providerString
     * @param $string
     * @param $token
     * @param $position
     * @param int $offset
     * @param int $limit
     */
    public function testSearch($string, $token, $position, $offset = 0, $limit = 0) {

        list($socket, $socket2) = Stream::pair();
        /* @var Stream $socket */
        /* @var Stream $socket2 */
        $socket->appendToInput($string);
        $this->assertEquals($position, $socket->search($token, $offset, $limit));
    }

    public function providerGets() {
        $string = "0123456789";
        return array(
            //     send     method      arguments for method                   read      tail
            0 => [$string, "get",     [5],                                 "01234", "56789"],
            1 => [$string, "get",     [10],                                "0123456789", ""],
            2 => [$string, "get",     [16],                                "0123456789", ""],

            3 => [$string, "getAll",  [],                                  $string, ""],

            4 => [$string, "getLine", ["3", Stream::MODE_TRIM_TOKEN, 8], "012", "456789"],
            5 => [$string, "getLine", ["3", Stream::MODE_WITH_TOKEN, 8], "0123", "456789"],
            6 => [$string, "getLine", ["3", Stream::MODE_WITHOUT_TOKEN, 8], "012", "3456789"],

            7 => [$string, "getLine", ["3", Stream::MODE_TRIM_TOKEN, 18], "012", "456789"],
            8 => [$string, "getLine", ["3", Stream::MODE_WITH_TOKEN, 18], "0123", "456789"],
            9 => [$string, "getLine", ["3", Stream::MODE_WITHOUT_TOKEN, 18], "012", "3456789"],

            10 => [$string, "getLine", ["9", Stream::MODE_TRIM_TOKEN, 4], false, $string],
            11 => [$string, "getLine", ["9", Stream::MODE_WITH_TOKEN, 4], false, $string],
            12 => [$string, "getLine", ["9", Stream::MODE_WITHOUT_TOKEN, 4], false, $string],

            13 => [$string, "getLine", ["01", Stream::MODE_TRIM_TOKEN, 8], "", "23456789"],
            14 => [$string, "getLine", ["01", Stream::MODE_WITH_TOKEN, 8], "01", "23456789"],
            15 => [$string, "getLine", ["01", Stream::MODE_WITHOUT_TOKEN, 8], "", "0123456789"],

            16 => [$string, "getLine", ["89", Stream::MODE_TRIM_TOKEN, 10], "01234567", ""],
            17 => [$string, "getLine", ["89", Stream::MODE_WITH_TOKEN, 10], "0123456789", ""],
            18 => [$string, "getLine", ["89", Stream::MODE_WITHOUT_TOKEN, 10], "01234567", "89"],

            19 => [$string, "getLine", ["45", Stream::MODE_TRIM_TOKEN], "0123", "6789"],
            20 => [$string, "getLine", ["45", Stream::MODE_WITH_TOKEN], "012345", "6789"],
            21 => [$string, "getLine", ["45", Stream::MODE_WITHOUT_TOKEN], "0123", "456789"],

            22 => [$string, "getLine", ["a", Stream::MODE_TRIM_TOKEN], false, $string],
            23 => [$string, "getLine", ["a", Stream::MODE_WITH_TOKEN], false, $string],
            24 => [$string, "getLine", ["a", Stream::MODE_WITHOUT_TOKEN], false, $string],
        );
    }

    /**
     * @memcheck
     * @dataProvider providerGets
     * @param string $string
     * @param string $method
     * @param array $args
     * @param string $result
     * @param string $tail
     * @group testGets
     */
    public function testGets($string, $method, $args, $result, $tail) {
        list($socket, $socket2) = Stream::pair();
        /* @var Stream $socket */
        /* @var Stream $socket2 */
        $socket->appendToInput($string);
        $this->assertEquals(strlen($string), $socket->getSize());
        $this->assertEquals($result, call_user_func_array([$socket, $method], $args));
        $this->assertEquals(strlen($tail), $socket->getSize());
        $this->assertEquals($tail, $socket->getAll());
    }

    public function chunkSender(array $chunks) {
        return function (Stream $connect) use ($chunks) {
            foreach($chunks as $chunk) {
                $connect->write($chunk);
                yield ION::await(self::SERVER_CHUNK_INTERVAL);
            }
            yield ION::await(self::SERVER_AWAIT_AFTER_ALL);
            $connect->close();
        };
    }

    public function listener($ip, callable $callback) {
        $listener = new Listener("tcp://".$ip);
        $listener->onConnect($callback)->onFail(function ($error) {
            $this->data["listener.error"] = $this->describe($error);
        });
        return $listener;
    }

    public function promise(callable $action, $stop = true) {
        ION::promise($action)
        ->then(function ($result) use ($stop) {
            if($result !== null) {
                $this->data["result"] = $this->describe($result);
            }
            if($stop) {
                $this->stop();
            }
        }, function ($error) use ($stop) {
            $this->data["error"] = $this->describe($error);
            if($stop) {
                $this->stop();
            }
        });
    }

    /**
     * @memcheck
     */
    public function testAwaitConnection() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $connect->write("1234");
            yield ION::await(0.03);
        });
        $listener->disable();
        $this->promise(function () use ($listener) {
            yield ION::await(0.06);
            $listener->enable();
        }, false);
        $this->promise(function () {
            $sock = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
            $this->data["pre_connect"] = $sock->isConnected();
            yield $sock->awaitConnection();
            $this->data["post_connect"] = $sock->isConnected();
            $this->data["await_again"] = $sock->awaitConnection();
            yield ION::await(0.1);
        });
        $this->loop();
        $this->assertEquals([
            "pre_connect" => false,
            "post_connect" => true,
            "await_again" => true,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testAwaitConnectionFail() {
        $this->promise(function () {
            $sock = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
            $this->data["pre_connect"] = $sock->isConnected();
            yield $sock->awaitConnection();
            $this->data["post_connect"] = $sock->isConnected();
            yield ION::await(0.1);
        });
        $this->loop();
        $this->assertEquals([
            "error" => [
                'exception' => 'ION\Stream\ConnectionException',
                'message'   => 'Connection refused',
                'code'      => 61
            ],
            "pre_connect" => false,
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testClose() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $connect->write("1234");
            yield ION::await(0.05);
            $connect->close();
        });
        $this->promise(function () {
            $sock = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
            yield ION::await(0.1);
            $this->data["is_closed"] = $sock->isClosed();
            $this->data["tail"] = $sock->getAll();
        });
        $this->loop();
        $listener->shutdown();
        $this->assertEquals([
            "is_closed" => true,
            "tail" => "1234",
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testFlush() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $connect->write("1234");
            yield $connect->flush();
            $connect->close();
        });
        $this->promise(function () {
            $sock = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
            yield ION::await(0.05);
            return $sock->getAll();
        });
        $this->loop();
        $this->assertEquals([
            "result" => "1234",
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testAwaitShutdown() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, $this->chunkSender(["01234", "56789"]));
        $this->promise(function () {
            $sock = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
            $this->data["closed"] = yield $sock->awaitShutdown();
            return $sock->getAll();
        });
        $this->loop();
        $this->assertEquals([
            "result" => "0123456789",
            "closed" => Stream::STATE_EOF
        ], $this->data);
    }

    public function providerReads() {
        $chunks = ["0123", "456", "789"];
        $string = implode("", $chunks);
        return array(
            //     send     method      arguments for method                     read      tail
            0 => [$chunks, "read", [5], "01234", "56789"],
            1 => [$chunks, "read", [10], $string, ""],
            2 => [$chunks, "read", [16], new ConnectionException('Connection was closed: received EOF', 0)],

            3 => [$chunks, "readAll", [], $string, ""],

            4 => [$chunks, "readLine", ["3", Stream::MODE_TRIM_TOKEN, 8], "012", "456789"],
            5 => [$chunks, "readLine", ["3", Stream::MODE_WITH_TOKEN, 8], "0123", "456789"],
            6 => [$chunks, "readLine", ["3", Stream::MODE_WITHOUT_TOKEN, 8], "012", "3456789"],

            7 => [$chunks, "readLine", ["3", Stream::MODE_TRIM_TOKEN, 18], "012", "456789"],
            8 => [$chunks, "readLine", ["3", Stream::MODE_WITH_TOKEN, 18], "0123", "456789"],
            9 => [$chunks, "readLine", ["3", Stream::MODE_WITHOUT_TOKEN, 18], "012", "3456789"],

            10 => [$chunks, "readLine", ["9", Stream::MODE_TRIM_TOKEN, 4], false, $string],
            11 => [$chunks, "readLine", ["9", Stream::MODE_WITH_TOKEN, 4], false, $string],
            12 => [$chunks, "readLine", ["9", Stream::MODE_WITHOUT_TOKEN, 4], false, $string],

            13 => [$chunks, "readLine", ["01", Stream::MODE_TRIM_TOKEN, 8], "", "23456789"],
            14 => [$chunks, "readLine", ["01", Stream::MODE_WITH_TOKEN, 8], "01", "23456789"],
            15 => [$chunks, "readLine", ["01", Stream::MODE_WITHOUT_TOKEN, 8], "", "0123456789"],

            16 => [$chunks, "readLine", ["89", Stream::MODE_TRIM_TOKEN, 10], "01234567", ""],
            17 => [$chunks, "readLine", ["89", Stream::MODE_WITH_TOKEN, 10], "0123456789", ""],
            18 => [$chunks, "readLine", ["89", Stream::MODE_WITHOUT_TOKEN, 10], "01234567", "89"],

            19 => [$chunks, "readLine", ["45", Stream::MODE_TRIM_TOKEN], "0123", "6789"],
            20 => [$chunks, "readLine", ["45", Stream::MODE_WITH_TOKEN], "012345", "6789"],
            21 => [$chunks, "readLine", ["45", Stream::MODE_WITHOUT_TOKEN], "0123", "456789"],

            22 => [$chunks, "readLine", ["a", Stream::MODE_TRIM_TOKEN], false, $string],
            23 => [$chunks, "readLine", ["a", Stream::MODE_WITH_TOKEN], false, $string],
            24 => [$chunks, "readLine", ["a", Stream::MODE_WITHOUT_TOKEN], false, $string],
        );
    }

    /**
     * @memcheck
     * @group dev
     * @dataProvider providerReads
     * @param array $chunks
     * @param string $method
     * @param array $args
     * @param string $result
     * @param string $tail
     */
    public function testReads($chunks, $method, $args, $result, $tail = null) {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, $this->chunkSender($chunks));
        $this->promise(function () use ($method, $args) {
            $sock = Stream::socket("tcp://".ION_TEST_SERVER_IPV4);
            $this->data["read"] = yield call_user_func_array([$sock, $method], $args);
//            $this->data["read"] = yield $sock->{$method}(...$args);
//            yield $sock->awaitShutdown();
//            $this->data["tail"] = $sock->getAll();
            yield ION::await(0.1);
        });

        $this->loop();

//        if($result instanceof \Exception) {
//            $this->assertEquals([
//                "error" => $this->describe($result)
//            ], $this->data);
//        } else {
//            $this->assertEquals([
//                "read" => $result,
//                "tail" => $tail,
//            ], $this->data);
//        }
    }

    /**
     *
     * Check memory leaks in the __debugInfo()
     * @memcheck
     */
    public function _testDebugInfo() {
        $pid = $this->setupSendServer(["01234","56789"]);
        $socket = Stream::socket(ION_TEST_SERVER_HOST);
        usleep(1e4); // time to ack
        $socket->__debugInfo();
        $socket->readLine("a");
        $socket->__debugInfo();
        $socket->awaitShutdown()->then(function (Stream $stream) {
            $stream->__debugInfo();
            $this->stop();
        });

        $this->loop();
        $this->assertWaitPID($pid);
    }

    /**
     * @memcheck
     */
    public function _testDestructWhileReading() {
        $pid = $this->setupSendServer(false);
        $socket = Stream::socket(ION_TEST_SERVER_HOST);
        $socket->await(2)->then(function() {
            $this->stop("then");
        });
        unset($socket);
        $this->assertWaitPID($pid);
        $this->kill($pid);
    }

    /**
     * @memcheck
     */
    public function _testGetLocalName() {
        $pid = $this->setupSendServer(false);
        $socket = Stream::socket(ION_TEST_SERVER_HOST);
        $hostname = strstr(ION_TEST_SERVER_HOST, ":", true);
        $this->assertStringMatchesFormat("{$hostname}:%d", $socket->getLocalName());
        $this->assertWaitPID($pid);
    }

    /**
     *
     * @memcheck
     */
    public function _testGetPeerName() {
        $pid = $this->setupSendServer(false);
        $socket = Stream::socket(ION_TEST_SERVER_HOST);
        usleep(1e4); // time to ack
        $this->assertStringMatchesFormat(ION_TEST_SERVER_HOST, $socket->getRemotePeer());
        $this->assertWaitPID($pid);
    }

    /**
     * @memcheck
     */
    public function _testToString() {
        $pid = $this->setupSendServer(["01234","56789"]);
        $hostname = strstr(ION_TEST_SERVER_HOST, ":", true);
        $host = ION_TEST_SERVER_HOST;
        $socket = Stream::socket(ION_TEST_SERVER_HOST);
        usleep(1e4); // time to ack
        $this->assertStringMatchesFormat("stream:socket({$hostname}:%d->{$host})", strval($socket));
        $this->assertWaitPID($pid);
    }
}
