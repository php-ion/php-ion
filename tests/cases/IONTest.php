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
}