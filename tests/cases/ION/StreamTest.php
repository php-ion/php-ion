<?php

namespace ION;

use ION;
use ION\Test\TestCase;

class StreamTest extends TestCase {

    public function setupServer($data, $timeout = TestCase::WORKER_DELAY) {
        return $this->listen(ION_TEST_SERVER_HOST)->inWorker($timeout)->onConnect(function ($connect) use ($data) {
            if(is_array($data)) {
                foreach ($data as $chunk) {
                    fwrite($connect, $chunk);
                    usleep(self::SERVER_CHUNK_INTERVAL); // 0.1s
                }
            } else {
                fwrite($connect, $data);
            }
        })->start();
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
        $pid = $this->listen(ION_TEST_SERVER_HOST)->inWorker()->onConnect(function ($connect) {
//			$this->out("Accept connection");
        })->start();

        $stream = Stream::socket(ION_TEST_SERVER_HOST);
        $this->assertInstanceOf('ION\Stream', $stream);
    }

    /**
     * @memcheck
     */
    public function testEnableDisable() {
        list($a, $b) = Stream::pair();
        /* @var Stream $a */
        /* @var Stream $b */
        $a->enable()->disable()->enable();
        $b->enable()->disable()->enable();
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
     *
     * @memcheck
     * @dataProvider providerString
     * @param $string
     * @param $token
     * @param $position
     * @param int $offset
     * @param int $limit
     */
    public function testSearch($string, $token, $position, $offset = 0, $limit = 0) {
        $pid = $this->setupServer($string);

        $socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();
        ION::await(0.03)->then(function () use ($socket, $token, $offset, $limit) {
            $this->data['search'] = $socket->search($token, $offset, $limit);
            $this->stop();
        });
        $this->loop();

        $this->assertEquals($position, $this->data['search']);
        $this->assertWaitPID($pid);
    }

    public function providerGets() {
        $string = "0123456789";
        return array(
            //     send     method      arguments for method                   read      tail
            0 => [$string, "get", [5], "01234", "56789"],
            1 => [$string, "get", [10], "0123456789", ""],
            2 => [$string, "get", [16], "0123456789", ""],

            3 => [$string, "getAll", [], $string, ""],

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
        $pid = $this->setupServer($string);
        $socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();
        ION::await(0.03)->then(function () use ($socket, $method, $args) {
            $this->data["size"] = $socket->getSize();
            $this->data[$method] = call_user_func_array([$socket, $method], $args);
            $this->data["tail_size"] = $socket->getSize();
            $this->data["tail"] = $socket->getAll();

            $this->stop();
        });
        $this->loop();
        $this->assertEquals([
            "size" => strlen($string),
            $method => $result,
            "tail" => $tail,
            "tail_size" => strlen($tail)
        ], $this->data);
        $this->assertWaitPID($pid);
    }

    /**
     * @group testAwaitShutdown
     * @memcheck
     */
    public function testAwaitShutdown() {
        $pid = $this->setupServer(["01234","56789"]);
        $socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();

        $socket->awaitShutdown()->then(function (Stream $stream, $error) {
            $this->assertNull($error);
            $this->assertInstanceOf('ION\Stream', $stream);
            $this->data["tail"]   = $stream->getAll();
            $this->data["closed"] = $stream->isClosed();
            $this->stop();
        });
        $this->loop();
        $this->assertEquals([
            "tail" => "0123456789",
            "closed" => Stream::STATE_EOF
        ], $this->data);
        $this->assertWaitPID($pid);
    }

    public function providerAwaits() {
        $chunks = ["0123", "456", "789"];
        $string = implode("", $chunks);
        return array(
            //     send     method      arguments for method                     read      tail
            0 => [$chunks, "await", [5], "01234", "56789"],
            1 => [$chunks, "await", [10], $string, ""],
            2 => [$chunks, "await", [16], $string, ""],

            3 => [$chunks, "awaitAll", [], $string, ""],

            4 => [$chunks, "awaitLine", ["3", Stream::MODE_TRIM_TOKEN, 8], "012", "456789"],
            5 => [$chunks, "awaitLine", ["3", Stream::MODE_WITH_TOKEN, 8], "0123", "456789"],
            6 => [$chunks, "awaitLine", ["3", Stream::MODE_WITHOUT_TOKEN, 8], "012", "3456789"],

            7 => [$chunks, "awaitLine", ["3", Stream::MODE_TRIM_TOKEN, 18], "012", "456789"],
            8 => [$chunks, "awaitLine", ["3", Stream::MODE_WITH_TOKEN, 18], "0123", "456789"],
            9 => [$chunks, "awaitLine", ["3", Stream::MODE_WITHOUT_TOKEN, 18], "012", "3456789"],

            10 => [$chunks, "awaitLine", ["9", Stream::MODE_TRIM_TOKEN, 4], false, $string],
            11 => [$chunks, "awaitLine", ["9", Stream::MODE_WITH_TOKEN, 4], false, $string],
            12 => [$chunks, "awaitLine", ["9", Stream::MODE_WITHOUT_TOKEN, 4], false, $string],

            13 => [$chunks, "awaitLine", ["01", Stream::MODE_TRIM_TOKEN, 8], "", "23456789"],
            14 => [$chunks, "awaitLine", ["01", Stream::MODE_WITH_TOKEN, 8], "01", "23456789"],
            15 => [$chunks, "awaitLine", ["01", Stream::MODE_WITHOUT_TOKEN, 8], "", "0123456789"],

            16 => [$chunks, "awaitLine", ["89", Stream::MODE_TRIM_TOKEN, 10], "01234567", ""],
            17 => [$chunks, "awaitLine", ["89", Stream::MODE_WITH_TOKEN, 10], "0123456789", ""],
            18 => [$chunks, "awaitLine", ["89", Stream::MODE_WITHOUT_TOKEN, 10], "01234567", "89"],

            19 => [$chunks, "awaitLine", ["45", Stream::MODE_TRIM_TOKEN], "0123", "6789"],
            20 => [$chunks, "awaitLine", ["45", Stream::MODE_WITH_TOKEN], "012345", "6789"],
            21 => [$chunks, "awaitLine", ["45", Stream::MODE_WITHOUT_TOKEN], "0123", "456789"],

            22 => [$chunks, "awaitLine", ["a", Stream::MODE_TRIM_TOKEN], false, $string],
            23 => [$chunks, "awaitLine", ["a", Stream::MODE_WITH_TOKEN], false, $string],
            24 => [$chunks, "awaitLine", ["a", Stream::MODE_WITHOUT_TOKEN], false, $string],
        );
    }

    /**
     * @memcheck
     *
     * @group awaits
     * @dataProvider providerAwaits
     * @param array $chunks
     * @param string $method
     * @param array $args
     * @param string $result
     * @param string $tail
     */
    public function testAwaits($chunks, $method, $args, $result, $tail) {
        $pid = $this->setupServer($chunks);

        $this->data = [];

        $socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();
        $socket->awaitShutdown()->then(function (Stream $socket) {
            $this->data["tail"] = $socket->getAll();
            $this->stop();
        });
        $deferred = call_user_func_array([$socket, $method], $args);
        $this->assertInstanceOf('ION\Deferred', $deferred);
        /** @var Deferred $deferred */
        $deferred->then(function ($data, $error) use ($socket) {
            $this->data["result"] = $data;
            $this->data["error"] = $error;
        });

        $this->loop();

        $this->assertEquals([
            "result" => $result,
            "error" => null,
            "tail" => $tail,
        ], $this->data);

		$this->assertWaitPID($pid);
    }

    /**
     * @memcheck
     */
    public function testDebugInfo() {
        $pid = $this->setupServer(["01234","56789"]);
        $socket = Stream::socket(ION_TEST_SERVER_HOST)->enable();
        $socket->__debugInfo();
        $socket->awaitLine("a");
        $socket->__debugInfo();
        $socket->awaitShutdown()->then(function (Stream $stream, $error) {
            $stream->__debugInfo();
            $this->stop();
        });

        $this->loop();
        $this->assertWaitPID($pid);
    }
}