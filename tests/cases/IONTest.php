<?php

use ION\Test\TestCase;

class IONTest extends TestCase {

	public function testAbbr() {
		$abbrs = [
			"Input-Output Notifications",
			"Input, Output, Notifications",
			"Input-Output, Notifications",
			"Input, Output Notifications",
		];

		foreach($abbrs as $abbr) {
			$this->assertEquals(1, preg_match('/^(I)nput(?:, |-)(O)utput,? (N)otifications$/i', $abbr, $matches));
			$this->assertEquals("ION", $matches[1].$matches[2].$matches[3]);
		}

	}

    /**
     * @memcheck
     */
    public function testAwait() {
        ION::await(0.5)->then(function() {
            ION::stop();
        });
        ION::dispatch();
    }
}