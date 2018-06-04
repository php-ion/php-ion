<?php

namespace ION\Test;


use PHPUnit\Exception;
use PHPUnit\Framework\AssertionFailedError;
use PHPUnit\Framework\Test;
use PHPUnit\Framework\TestSuite;
use PHPUnit\Framework\Warning;
use PHPUnit\Runner\PhptTestCase;
use PHPUnit\TextUI\ResultPrinter;

class Printer extends ResultPrinter {


    public function describe(Test $test): string
    {
        [$class, $name] = \PHPUnit\Util\Test::describe($test);
        if ($class) {
            return $class . ": " . $name;
        } else {
            return $name;
        }
    }

    public function describeException(\Throwable $e): string
    {
        $message = $e->getMessage();
        if (strpos($message, "\n")) {
            $message = trim(strstr($message, "\n", true));
        }
        if (!$e instanceof Exception) {
            $message .= "(" . get_class($e) . ")";
        }

        return $message;
    }

    /**
     * An error occurred.
     *
     * @param Test        $test
     * @param \Throwable  $e
     * @param float       $time
     */
    public function addError(Test $test, \Throwable $e, float $time): void
    {
        $this->writeProgressWithColor('fg-red, bold', sprintf(
            "\rERR  %d/%d %s: %s",
            $this->numTestsRun,
            $this->numTests,
            $this->describe($test),
            $this->describeException($e)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * A failure occurred.
     *
     * @param Test                 $test
     * @param AssertionFailedError $e
     * @param float                $time
     */
    public function addFailure(Test $test, AssertionFailedError $e, float $time): void
    {
        $this->writeProgressWithColor('fg-red', sprintf(
            "\rFAIL %d/%d %s",
            $this->numTestsRun,
            $this->numTests,
            $this->describe($test)
        ));
        $this->lastTestFailed = true;
    }

	/**
	 * A warning occurred.
	 *
	 * @param Test    $test
	 * @param Warning $e
	 * @param float   $time
	 *
	 * @since Method available since Release 5.1.0
	 */
	public function addWarning(Test $test, Warning $e, float $time): void
	{
		$this->writeProgressWithColor('fg-yellow, bold', sprintf(
			"\rWARN %d/%d %s: %s",
			$this->numTestsRun,
			$this->numTests,
            $this->describe($test),
            $this->describeException($e)
		));
		$this->lastTestFailed = true;
	}

    /**
     * Incomplete test.
     *
     * @param Test       $test
     * @param \Throwable $e
     * @param float      $time
     */
    public function addIncompleteTest(Test $test, \Throwable $e, float $time): void
    {
        $this->writeProgressWithColor('fg-yellow, bold', sprintf(
            "\rINC  %d/%d %s: %s",
            $this->numTestsRun,
            $this->numTests,
            $this->describe($test),
            $this->describeException($e)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * Risky test.
     *
     * @param Test       $test
     * @param \Throwable $e
     * @param float      $time
     *
     * @since  Method available since Release 4.0.0
     */
    public function addRiskyTest(Test $test, \Throwable $e, float $time): void
    {
        $this->writeProgressWithColor('fg-yellow, bold', sprintf(
            "\rRISK %d/%d %s: %s",
            $this->numTestsRun,
            $this->numTests,
            $this->describe($test),
            $this->describeException($e)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * Skipped test.
     *
     * @param Test       $test
     * @param \Throwable $e
     * @param float      $time
     *
     * @since  Method available since Release 3.0.0
     */
    public function addSkippedTest(Test $test, \Throwable $e, float $time): void
    {
        $this->writeProgressWithColor('fg-cyan, bold', sprintf(
            "\rSKIP %d/%d %s: %s",
            $this->numTestsRun,
            $this->numTests,
            $this->describe($test),
            $this->describeException($e)
        ));
        $this->lastTestFailed = true;
    }

    /**
     * A testsuite started.
     *
     * @param TestSuite $suite
     *
     * @since  Method available since Release 2.2.0
     */
    public function startTestSuite(TestSuite $suite): void
    {
        if ($this->numTests == -1) {
            $this->numTests      = count($suite);
        }
    }

    /**
     * A test started.
     *
     * @param Test $test
     */
    public function startTest(Test $test): void
    {
        $this->numTests;
        $this->write(sprintf(
            "Test %d/%d %s",
            ++$this->numTestsRun,
            $this->numTests,
            $this->describe($test)
        ));
    }

    /**
     * A test ended.
     *
     * @param Test  $test
     * @param float $time
     */
    public function endTest(Test $test, float $time): void
    {
        if (!$this->lastTestFailed) {
            $this->writeProgress(sprintf(
                "\rOK   %d/%d %s",
                $this->numTestsRun,
                $this->numTests,
                $this->describe($test)
            ));
        }

        if ($test instanceof \PHPUnit\Framework\TestCase) {
            $this->numAssertions += $test->getNumAssertions();
        } elseif ($test instanceof PhptTestCase) {
            $this->numAssertions++;
        }

        $this->lastTestFailed = false;

        if ($test instanceof \PHPUnit\Framework\TestCase) {
            if (!$test->hasExpectationOnOutput()) {
                $this->write($test->getActualOutput());
            }
        }
    }


    /**
     * @param string $progress
     */
    protected function writeProgress(string $progress): void
    {
        $this->write($progress);
//        $this->numTestsRun++;
        $this->writeNewLine();
    }
}