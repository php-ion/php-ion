<?php
namespace ION\Test;

use Exception;
use PHPUnit_Util_Test;
use PHPUnit_Util_Printer;
use PHPUnit_Framework_Test;
use PHPUnit_Framework_TestCase;
use PHPUnit_Framework_TestSuite;
use PHPUnit_Framework_Exception;
use Symfony\Component\Yaml\Dumper;
use PHPUnit_Framework_TestFailure;
use PHPUnit_Framework_TestListener;
use PHPUnit_Framework_AssertionFailedError;
use PHPUnit_Framework_ExpectationFailedException;

/**
 * A TestListener that generates a logfile of the
 * test execution using the Extended Test Anything Protocol (ETAP).
 *
 */
class ETAP extends PHPUnit_Util_Printer implements PHPUnit_Framework_TestListener {
    /**
     * @var int
     */
    protected $testNumber = 0;

    /**
     * @var int
     */
    protected $testSuiteLevel = 0;

    /**
     * @var bool
     */
    protected $testSuccessful = true;

    /**
     * Constructor.
     *
     * @param  mixed $out
     * @throws PHPUnit_Framework_Exception
     * @since  Method available since Release 3.3.4
     */
    public function __construct($out = null) {
        parent::__construct($out);
        $this->write("ETAP version 1 (TAP version 13)\n");
    }

    /**
     * An error occurred.
     *
     * @param PHPUnit_Framework_Test $test
     * @param Exception $e
     * @param float $time
     */
    public function addError(PHPUnit_Framework_Test $test, Exception $e, $time) {
        $this->writeNotOk($test, 'Error');
        $this->write(get_class($e) . ": " . $e->getMessage() . " in " . $e->getFile() . ":" . $e->getLine() . "\n" . $e->getTraceAsString());
    }

    /**
     * A failure occurred.
     *
     * @param PHPUnit_Framework_Test $test
     * @param PHPUnit_Framework_AssertionFailedError $e
     * @param float $time
     */
    public function addFailure(PHPUnit_Framework_Test $test, PHPUnit_Framework_AssertionFailedError $e, $time) {
        $this->writeNotOk($test, 'Failure');

        $message = explode(
            "\n",
            PHPUnit_Framework_TestFailure::exceptionToString($e)
        );

        $diagnostic = array(
            'message' => $message[0],
            'severity' => 'fail'
        );

        if ($e instanceof PHPUnit_Framework_ExpectationFailedException) {
            $cf = $e->getComparisonFailure();

            if ($cf !== null) {
                $diagnostic['data'] = array(
                    'got' => $cf->getActual(),
                    'expected' => $cf->getExpected()
                );
            }
        }

        $yaml = new Dumper;

        $this->write(
            sprintf(
                "  ---\n%s  ...\n",
                $yaml->dump($diagnostic, 2, 2)
            ) . "\n" . $e->getTraceAsString()
        );
    }

    /**
     * Incomplete test.
     *
     * @param PHPUnit_Framework_Test $test
     * @param Exception $e
     * @param float $time
     */
    public function addIncompleteTest(PHPUnit_Framework_Test $test, Exception $e, $time) {
        $this->writeNotOk($test, '', 'TODO Incomplete Test');
    }

    /**
     * Risky test.
     *
     * @param PHPUnit_Framework_Test $test
     * @param Exception $e
     * @param float $time
     * @since  Method available since Release 4.0.0
     */
    public function addRiskyTest(PHPUnit_Framework_Test $test, Exception $e, $time) {
        $this->write(
            sprintf(
                "\rok %d - # RISKY%s\n",
                $this->testNumber,
                $e->getMessage() != '' ? ' ' . $e->getMessage() : ''
            )
        );

        $this->testSuccessful = false;
    }

    /**
     * Skipped test.
     *
     * @param PHPUnit_Framework_Test $test
     * @param Exception $e
     * @param float $time
     * @since  Method available since Release 3.0.0
     */
    public function addSkippedTest(PHPUnit_Framework_Test $test, Exception $e, $time) {
        $this->write(
            sprintf(
                "\rok %d - # SKIP%s\n",
                $this->testNumber,
                $e->getMessage() != '' ? ' ' . $e->getMessage() : ''
            )
        );

        $this->testSuccessful = false;
    }

    /**
     * A testsuite started.
     *
     * @param PHPUnit_Framework_TestSuite $suite
     */
    public function startTestSuite(PHPUnit_Framework_TestSuite $suite) {
        $this->testSuiteLevel++;
    }

    /**
     * A testsuite ended.
     *
     * @param PHPUnit_Framework_TestSuite $suite
     */
    public function endTestSuite(PHPUnit_Framework_TestSuite $suite) {
        $this->testSuiteLevel--;

        if ($this->testSuiteLevel == 0) {
            $this->write(sprintf("1..%d\n", $this->testNumber));
        }
    }

    /**
     * A test started.
     *
     * @param PHPUnit_Framework_Test $test
     */
    public function startTest(PHPUnit_Framework_Test $test) {
        $this->testNumber++;
        $this->testSuccessful = true;
        $this->write(
            sprintf(
                "?? %d - %s ",
                $this->testNumber,
                PHPUnit_Util_Test::describe($test)
            )
        );
    }

    /**
     * A test ended.
     *
     * @param PHPUnit_Framework_Test $test
     * @param float $time
     */
    public function endTest(PHPUnit_Framework_Test $test, $time) {
        if ($this->testSuccessful === true) {
            $this->write(
                sprintf(
                    "\rok %d - %s\n",
                    $this->testNumber,
                    PHPUnit_Util_Test::describe($test)
                )
            );
        }

        $this->writeDiagnostics($test);
    }

    /**
     * @param PHPUnit_Framework_Test $test
     * @param string $prefix
     * @param string $directive
     */
    protected function writeNotOk(PHPUnit_Framework_Test $test, $prefix = '', $directive = '') {
        $this->write(
            sprintf(
                "\rnot ok %d - %s%s%s\n",
                $this->testNumber,
                $prefix != '' ? $prefix . ': ' : '',
                PHPUnit_Util_Test::describe($test),
                $directive != '' ? ' # ' . $directive : ''
            )
        );

        $this->testSuccessful = false;
    }

    /**
     * @param PHPUnit_Framework_Test $test
     */
    private function writeDiagnostics(PHPUnit_Framework_Test $test) {
        if (!$test instanceof PHPUnit_Framework_TestCase) {
            return;
        }

        if (!$test->hasOutput()) {
            return;
        }

        foreach (explode("\n", trim($test->getActualOutput())) as $line) {
            $this->write(
                sprintf(
                    "# %s\n",
                    $line
                )
            );
        }
    }
}
