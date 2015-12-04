<?php

namespace ION;

use ION;
use ION\Stream\ConnectionException;
use ION\Test\TestCase;

class StreamTest extends TestCase {

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
        $stream = Stream::socket(ION_TEST_SERVER_IPV4);
        $this->assertInstanceOf('ION\Stream', $stream);
    }

    public function providerSocketFailures() {
        return [
            ["tcp://127.0.0.1:80", "Failed to open socket tcp://127.0.0.1:80: No such directory"],
            ["?query", "Host ?query is not well-formed"],
        ];
    }

    /**
     * @dataProvider providerSocketFailures
     * @param string $url
     * @param string $message
     * @param int $code
     */
    public function testSocketFailures($url, $message, $code = 0) {
        try  {
            $stream = Stream::socket($url);
            $this->fail("Should be exception");
        } catch(\Exception $e) {
            $this->assertException($e, $message, $code);
        }
    }

    /**
     * @memcheck
     */
    public function testEnableDisable() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $this->assertTrue($a->isEnabled());
        $this->assertTrue($a->isConnected());
        $this->assertEquals(Stream::STATE_ENABLED, $a->getState() & Stream::STATE_ENABLED);
        $a->disable();
        $this->assertFalse($a->isEnabled());
        $this->assertTrue($a->isConnected());
        $this->assertEquals(0, $a->getState() & Stream::STATE_ENABLED);

        $a->enable();
        $b->disable()->disable()->enable()->enable();
    }

    /**
     * @memcheck
     */
    public function testPriority() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $a->setPriority(ION::PRIORITY_HIGH);
    }

    /**
     * @group dev
     * @memcheck
     */
    public function testInputBufferSize() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $a->setInputBufferSize(KiB);
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
            [$string, "a", -1],
            [$string, $string."a", -1],
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

            25 => [$string, "getLine", [$string."0", Stream::MODE_TRIM_TOKEN], false, $string],
            26 => [$string, "getLine", [$string."0", Stream::MODE_WITH_TOKEN], false, $string],
            27 => [$string, "getLine", [$string."0", Stream::MODE_WITHOUT_TOKEN], false, $string],
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
        $listener = new Listener($ip);
        $listener->accept()->then($callback)->onFail(function ($error) {
            $this->data["listener.error"] = $this->describe($error);
        });
        return $listener;
    }

    /**
     * @memcheck
     */
    public function testConnect() {
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
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
            $this->data["pre_connect"] = $sock->isConnected();
            yield $sock->connect();
            $this->data["post_connect"] = $sock->isConnected();
            $this->data["await_again"] = $sock->connect();
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
        $server_ip = ION_TEST_SERVER_IPV4;
        $client_ip = strstr(ION_TEST_SERVER_IPV4, ":", true);
        $this->promise(function () {
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
            $this->data["pre_connect"] = $sock->isConnected();
            yield $sock->connect();
            $this->data["post_connect"] = $sock->isConnected();
            yield ION::await(0.1);
        });
        $this->loop();
        $this->assertFalse($this->data['pre_connect']);
        $this->assertTrue(isset($this->data['error']));
        $this->assertEquals('ION\Stream\ConnectionException', $this->data['error']['exception']);
        $this->assertEquals(0, $this->data['error']['code']);
        $this->assertStringMatchesFormat("Stream({$client_ip}:%i->{$server_ip}) error: Connection refused", $this->data['error']['message']);
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
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
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
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
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
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
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
            2 => [$chunks, "read", [16], false, $string],

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
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
            $sock->__debugInfo();
            $this->data["read"] = yield $sock->{$method}(...$args);
            $sock->__debugInfo();
            yield $sock->awaitShutdown();
            $sock->__debugInfo();
            $this->data["tail"] = $sock->getAll();
            $sock->__debugInfo();
            yield ION::await(0.02);
            $sock->__debugInfo();
        });

        $this->loop();

        if($result instanceof \Exception) {
            $this->assertEquals([
                "error" => $this->describe($result)
            ], $this->data);
        } else {
            $this->assertEquals([
                "read" => $result,
                "tail" => $tail,
            ], $this->data);
        }
    }

    public function providerDoubleReads() {
        $chunks = ["0123", "456789"];
        $string = implode("", $chunks);
        return array(
            //     send     method      arguments for method
            0 => [$chunks, "read", [5], "01234"],
            3 => [$chunks, "readAll", [], $string],
            4 => [$chunks, "readLine", ["5", Stream::MODE_TRIM_TOKEN, 8], "01234"],
        );
    }

    /**
     * @memcheck
     * @dataProvider providerDoubleReads
     * @param array $chunks
     * @param string $method
     * @param array $args
     * @param string $result
     */
    public function testDoubleReads($chunks, $method, $args, $result) {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, $this->chunkSender($chunks));
        $this->promise(function () use ($method, $args) {
            $sock = Stream::socket(ION_TEST_SERVER_IPV4);
            $deferred = $sock->{$method}(...$args);
            $this->assertInstanceOf(Deferred::class, $deferred);
            $this->assertSame($deferred, $sock->{$method}(...$args));

            $this->data["read"] = yield $sock->{$method}(...$args);
            yield ION::await(0.02);
        });

        $this->loop();

        if($result instanceof \Exception) {
            $this->assertEquals([
                "error" => $this->describe($result)
            ], $this->data);
        } else {
            $this->assertEquals([
                "read" => $result,
            ], $this->data);
        }
    }

    /**
     * Check memory leaks in the __debugInfo()
     * @memcheck
     */
    public function testDebugInfo() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, $this->chunkSender(["01234"]));

        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            $socket->__debugInfo();
            yield $socket->connect();
            $socket->readLine("a");
            $socket->__debugInfo();
            $socket->close();
            $socket->__debugInfo();
            yield ION::await(0.1);
        });


        $this->loop();
    }

    /**
     * @memcheck
     */
    public function testGetNames() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $connect->write("1234");
            $this->data["server.remote"] = $connect->getPeerName();
            $this->data["server.local"] = $connect->getName();
            yield $connect->flush();
            $connect->close();
        });
        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            $this->data["client.remote.before"] = $socket->getPeerName();
            yield $socket->connect();
            $this->data["client.remote.after"] = $socket->getPeerName();
            $this->data["client.local"] = $socket->getName();
            yield ION::await(0.1);
        });

        $this->loop();

        $ip = strstr(ION_TEST_SERVER_IPV4, ":", true);
        $this->assertTrue(in_array($this->data["client.remote.before"], [false, ION_TEST_SERVER_IPV4])); // may be set or not
        $this->assertStringMatchesFormat("$ip:%i", $this->data["server.remote"]);
        $this->assertEquals(ION_TEST_SERVER_IPV4, $this->data["server.local"]);
        $this->assertEquals(ION_TEST_SERVER_IPV4, $this->data["client.remote.after"]);
        $this->assertStringMatchesFormat("$ip:%i", $this->data["client.local"]);
    }

    /**
     * @memcheck
     */
    public function testDestructMemoryLeak() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, $this->chunkSender(["01234"]));
        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            $socket->readLine("z");
            $socket->flush();
            $socket->connect();
            $socket->awaitShutdown();
            $socket->onData()->then(function () {});
            $socket->getName();
            $socket->getPeerName();
            unset($socket);
            yield ION::await(0.1);
        });

        $this->loop();
    }

    /**
     * @memcheck
     */
    public function testToString() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $connect->write("1234");
            $this->data["server.stream"] = strval($connect);
            yield $connect->flush();
            $connect->close();
        });
        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            yield $socket->connect();
            $this->data["client.stream"] = strval($socket);
            yield ION::await(0.1);
        });

        $this->loop();

        $ip = strstr(ION_TEST_SERVER_IPV4, ":", true);
        $server_addr = ION_TEST_SERVER_IPV4;
        $this->assertStringMatchesFormat("Stream($ip:%i->$server_addr)", $this->data["client.stream"]);
        $this->assertStringMatchesFormat("Stream($server_addr<-$ip:%i)", $this->data["server.stream"]);
    }

    /**
     * @memcheck
     */
    public function testOnData() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, $this->chunkSender(["0123", "4567"]));
        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            $socket->onData()->then(function (Stream $stream) {
                $this->data["data"] = $stream->getAll();
            });
            $this->data["read"] = yield $socket->read(5);
            yield ION::await(0.1);
        });

        $this->loop();

        $this->assertEquals([
            "read" => "01234",
            "data" => "567",
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testSendValidFile() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $this->data["sendfile"] = $connect->sendFile(__FILE__);
            yield $connect->flush();
            $connect->close();
        });
        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            $this->data["file"] = yield $socket->readAll();
            yield ION::await(0.02);
        });

        $this->loop();

        $this->assertTrue($this->data["sendfile"]);
        $this->assertStringEqualsFile(__FILE__, $this->data["file"]);
    }

    /**
     * @memcheck
     */
    public function testSendInvalidFile() {
        $listener = $this->listener(ION_TEST_SERVER_IPV4, function (Stream $connect) {
            $this->data["sendfile"] = $connect->sendFile('/unexist');
            yield $connect->flush();
            $connect->close();
        });
        $this->promise(function () {
            $socket = Stream::socket(ION_TEST_SERVER_IPV4);
            $this->data["file"] = yield $socket->readAll();
            yield ION::await(0.02);
        });

        $this->loop();

        $this->assertFalse($this->data["sendfile"]);
        $this->assertEmpty($this->data["file"]);
    }

    /**
     * @memcheck
     */
    public function testEncrypted() {
        $this->promise(function () {
            $socket = Stream::socket("example.com:443", SSL::client());
            $socket->write(implode("\r\n", ["GET / HTTP/1.1",
                "Host: example.com",
                "Connection: close",
                "Accept: text/html",
                "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.86 Safari/537.36",
                "Accept-Language: en-US,en;q=0.8,ru;q=0.6"])."\r\n\r\n");
            $this->data["response"] = yield $socket->readAll();
        });
        $this->loop(1);

        $this->assertTrue(isset($this->data["response"]));
        $this->assertEquals("HTTP/1.1 200 OK", strstr($this->data["response"], "\r\n", true));
    }

    public function testEnableEncryption() {

    }

    /**
     * @group dev
     * @memcheck
     */
    public function testExtends() {
        list($a, $b) = MyStream::pair();
        $this->assertInstanceOf(MyStream::class, $a);
        $this->assertInstanceOf(MyStream::class, $b);
        $this->assertTrue($a->custom);
        $this->assertTrue($b->custom);
    }
}

class MyStream extends Stream {
    public $custom = false;

    public function __construct() {
        $this->custom = true;
    }
}
