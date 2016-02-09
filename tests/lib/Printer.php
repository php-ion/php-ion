<?php

namespace ION\Test;


class Printer extends \PHPUnit_TextUI_ResultPrinter {


    /**
     * An error occurred.
     *
     * @param \PHPUnit_Framework_Test $test
     * @param \Exception              $e
     * @param float                  $time
     */
    public function addError(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
        $this->writeProgressWithColor('fg-red, bold', sprintf(
            "\rERR  %d/%d %s",
            $this->numTestsRun,
            $this->numTests,
            \PHPUnit_Util_Test::describe($test)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * A failure occurred.
     *
     * @param \PHPUnit_Framework_Test                 $test
     * @param \PHPUnit_Framework_AssertionFailedError $e
     * @param float                                  $time
     */
    public function addFailure(\PHPUnit_Framework_Test $test, \PHPUnit_Framework_AssertionFailedError $e, $time) {
        $this->writeProgressWithColor('fg-red', sprintf(
            "\rFAIL %d/%d %s",
            $this->numTestsRun,
            $this->numTests,
            \PHPUnit_Util_Test::describe($test)
        ));
        $this->lastTestFailed = true;
    }

	/**
	 * A warning occurred.
	 *
	 * @param \PHPUnit_Framework_Test    $test
	 * @param \PHPUnit_Framework_Warning $e
	 * @param float                     $time
	 *
	 * @since Method available since Release 5.1.0
	 */
	public function addWarning(\PHPUnit_Framework_Test $test, \PHPUnit_Framework_Warning $e, $time)
	{
		$this->writeProgressWithColor('fg-yellow, bold', sprintf(
			"\rWARN %d/%d %s",
			$this->numTestsRun,
			$this->numTests,
			\PHPUnit_Util_Test::describe($test)
		));
		$this->lastTestFailed = true;
	}

    /**
     * Incomplete test.
     *
     * @param \PHPUnit_Framework_Test $test
     * @param \Exception              $e
     * @param float                  $time
     */
    public function addIncompleteTest(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
        $this->writeProgressWithColor('fg-yellow, bold', sprintf(
            "\rINC  %d/%d %s",
            $this->numTestsRun,
            $this->numTests,
            \PHPUnit_Util_Test::describe($test)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * Risky test.
     *
     * @param \PHPUnit_Framework_Test $test
     * @param \Exception              $e
     * @param float                  $time
     *
     * @since  Method available since Release 4.0.0
     */
    public function addRiskyTest(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
        $this->writeProgressWithColor('fg-yellow, bold', sprintf(
            "\rRISK %d/%d %s",
            $this->numTestsRun,
            $this->numTests,
            \PHPUnit_Util_Test::describe($test)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * Skipped test.
     *
     * @param \PHPUnit_Framework_Test $test
     * @param \Exception              $e
     * @param float                  $time
     *
     * @since  Method available since Release 3.0.0
     */
    public function addSkippedTest(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
        $this->writeProgressWithColor('fg-cyan, bold', sprintf(
            "\rSKIP %d/%d %s",
            $this->numTestsRun,
            $this->numTests,
            \PHPUnit_Util_Test::describe($test)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * A testsuite started.
     *
     * @param \PHPUnit_Framework_TestSuite $suite
     *
     * @since  Method available since Release 2.2.0
     */
    public function startTestSuite(\PHPUnit_Framework_TestSuite $suite)
    {
        if ($this->numTests == -1) {
            $this->numTests      = count($suite);
        }
    }

    /**
     * A test started.
     *
     * @param \PHPUnit_Framework_Test $test
     */
    public function startTest(\PHPUnit_Framework_Test $test) {
        $this->numTests;
        $this->write(sprintf(
            "Test %d/%d %s",
            ++$this->numTestsRun,
            $this->numTests,
            \PHPUnit_Util_Test::describe($test)
        ));
    }

    /**
     * A test ended.
     *
     * @param \PHPUnit_Framework_Test $test
     * @param float                  $time
     */
    public function endTest(\PHPUnit_Framework_Test $test, $time)
    {
        if (!$this->lastTestFailed) {
            $this->writeProgress(sprintf(
                "\rOK   %d/%d %s",
                $this->numTestsRun,
                $this->numTests,
                \PHPUnit_Util_Test::describe($test)
            ));
        }

        if ($test instanceof \PHPUnit_Framework_TestCase) {
            $this->numAssertions += $test->getNumAssertions();
        } elseif ($test instanceof \PHPUnit_Extensions_PhptTestCase) {
            $this->numAssertions++;
        }

        $this->lastTestFailed = false;

        if ($test instanceof \PHPUnit_Framework_TestCase) {
            if (!$test->hasExpectationOnOutput()) {
                $this->write($test->getActualOutput());
            }
        }
    }


    /**
     * @param string $progress
     */
    protected function writeProgress($progress)  {
        $this->write($progress);
//        $this->numTestsRun++;
        $this->writeNewLine();
    }
}