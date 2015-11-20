<?php

use ION\Test\TestCase;

class IONTest extends TestCase {

    /**
     * Always should be fine
     * @memcheck
     */
    public function testMemcheck() {

    }

    public function testAbbr() {
        $abbrs = [
            "Input-Output Notifications",
            "Input, Output, Notifications",
            "Input-Output, Notifications",
            "Input, Output Notifications",
        ];

        foreach ($abbrs as $abbr) {
            $this->assertEquals(1, preg_match('/^(I)nput(?:, |-)(O)utput,? (N)otifications$/i', $abbr, $matches));
            $this->assertEquals("ION", $matches[1] . $matches[2] . $matches[3]);
        }

    }

    /**
     * @memcheck
     */
    public function testAwait() {
        $start = microtime(1);
        ION::await(0.3)->then(function ($result) use ($start) {
            $this->data["await"] = $result;
            $this->data["timer"] = round(microtime(1) - $start, 1);
            $this->stop();
        });

        $this->loop();
        $this->assertEquals([
            "await" => true,
            "timer" => 0.3
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testAwaitAwait() {
        $start = microtime(1);
        ION::await(0.1)->then(function () use ($start) {
            yield ION::await(0.1);
            $this->data["timer"] = round(microtime(1) - $start, 1);
            $this->stop();
        });
        $this->loop();
        $this->assertEquals([
            "timer" => 0.2
        ], $this->data);
    }

    /**
     * @memcheck
     */
    public function testInterval() {
        $this->data["count"] = 0;
        $start = microtime(1);
        ION::interval(0.3, "test")->then(function () use ($start) {
            $this->data["iteration#".$this->data["count"]] = round(microtime(1) - $start, 1);
            if(++$this->data["count"] > 2) {
                $this->stop();
            }
        });
        $this->loop();
        $this->assertTrue(ION::cancelInterval("test"));
        $this->assertEquals([
            "count" => 3,
            "iteration#0" => 0.3,
            "iteration#1" => 0.6,
            "iteration#2" => 0.9,
        ], $this->data);
    }

    public function providerPromise() {
        $promise = new \ION\ResolvablePromise();
        $promise->done("promise_result");
        return array(
            ["result", "result"],
            [$promise, "promise_result"],
            [
                function () {
                    return "cb_result";
                },
                "cb_result"
            ],
            [
                function () {
                    yield ION::await(0.1);
                    return "yield_result";
                },
                "yield_result"
            ],
            [
                function () {
                    return ION::await(0.1);
                },
                true
            ]
        );
    }

    /**
     * @memcheck
     * @dataProvider providerPromise
     * @param mixed $resolver
     * @param mixed $result
     */
    public function testPromise($resolver, $result) {
        $promise = ION::promise($resolver);
        $this->assertInstanceOf('ION\Promise', $promise);
        $promise->then(function ($data) {
            $this->data["result"] = $this->describe($data);
            $this->stop();
        }, function ($error) {
            $this->data["error"] = $this->describe($error);
            $this->stop();

        });
        $this->loop(0.3, false);
        $this->assertEquals([
            "result" => $result
        ], $this->data);
    }

	/**
	 * @memcheck
	 */
	public function testReinit() {
		$this->assertTrue(ION::reinit());
	}
}