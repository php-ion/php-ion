<?php
namespace ION;

use ION\Process;
use ION\Process\Signal as Sig;
use ION\Test\TestCase;

/**
 * Test ION\Process class
 * @see stubs/ION/Process.php
 */
class ProcessTest extends TestCase {

    /**
     *
     * @memcheck
     */
    public function testPids() {
        $this->assertSame(getmypid(), Process::getPid());
        $this->assertSame(posix_getppid(), Process::getParentPid());
    }

	/**
	 * @memcheck
	 */
	public function testSignals() {
		Process::signal(SIGHUP)->then(function ($signo) {
			$this->data["signal"] = $signo;
		})->setName("test");

		$this->promise(function () {
			yield \ION::await(0.01);
			Process::kill(SIGHUP, getmypid());
			yield \ION::await(0.01);
		});
		$this->loop();
        Process::signal(SIGHUP)->forget('test');
		$this->assertEquals([
			"signal" => SIGHUP
		], $this->data);
	}

    /**
     * @memcheck
     */
    public function testGetUser() {
        $actual = Process::getUser();
        $expected = posix_getpwnam(posix_getlogin());
        if(!$expected) {
            $this->markTestSkipped("posix_getpwnam() failed (root?)");
        }
        $this->assertSame($expected['name'], $actual['name']);
        $this->assertSame($expected['uid'], $actual['uid']);
        $this->assertSame($expected['gid'], $actual['gid']);
        $this->assertSame($expected['dir'], $actual['dir']);
        $this->assertSame($expected['shell'], $actual['shell']);
    }

    /**
     * @memcheck
     */
    public function testGetAnotherUser() {
        $actual = Process::getUser('nobody');
        $expected = posix_getpwnam('nobody');
        $this->assertEquals($expected['name'], $actual['name']);
        $this->assertEquals($expected['gecos'], $actual['gecos']);
        $this->assertSame($expected['uid'], $actual['uid']);
        $this->assertSame($expected['gid'], $actual['gid']);
        $this->assertSame($expected['dir'], $actual['dir']);
        $this->assertSame($expected['shell'], $actual['shell']);
    }

    /**
     * @memcheck
     */
    public function testGetAnotherUserUID() {
        $actual = Process::getUser(intval(`id -u nobody`));
        $expected = posix_getpwnam('nobody');
        $this->assertSame($expected['name'], $actual['name']);
        $this->assertSame($expected['uid'], $actual['uid']);
        $this->assertSame($expected['gid'], $actual['gid']);
        $this->assertSame($expected['dir'], $actual['dir']);
        $this->assertSame($expected['shell'], $actual['shell']);
    }

    /**
     * @memcheck
     */
    public function testGetGroup() {
        $actual = Process::getGroup();
        $expected = posix_getgrgid(posix_getgid());
        if(!$expected) {
            $this->markTestSkipped("posix_getgrgid() failed (root?)");
        }
        $this->assertSame($expected['name'], $actual['name']);
        $this->assertSame($expected['gid'], $actual['gid']);
        $this->assertSame($expected['members'], $actual['members']);
    }

    /**
     * @memcheck
     */
    public function testGetAnotherGroup() {
        $actual = Process::getGroup('nobody');
        $expected = posix_getgrnam('nobody');
        if(!$expected) {
            $this->markTestSkipped("posix_getgrnam() failed (root?)");
        }
        $this->assertSame($expected['name'], $actual['name']);
        $this->assertSame($expected['gid'], $actual['gid']);
        $this->assertSame($expected['members'], $actual['members']);
    }

    /**
     * @memcheck
     */
    public function testFork() {
        $pid = Process::fork();
        if ($pid) {
            $this->assertTrue(posix_kill($pid, 0));
        } else {
            usleep(10000);
            exit(0);
        }
    }

    /**
     * @memcheck
     */
    public function testGetPriority() {
        $this->assertSame(pcntl_getpriority(), Process::getPriority());
        $this->assertSame(pcntl_getpriority(posix_getppid()), Process::getPriority(posix_getppid()));
    }


    /**
     * @memcheck
     */
    public function testSetPriority() {
        $prio = Process::getPriority();
        $pid = pcntl_fork();
        if ($pid) {
            usleep(10000);
            $this->assertSame($prio + 10, Process::getPriority($pid));
            $this->assertWaitPID($pid);
        } else {
            Process::setPriority(10);
            usleep(20000);
            exit(0);
        }
    }

    /**
     * @memcheck
     */
    public function _testSetUser() {
        Process::setUser('www', true);
        var_dump(Process::getUser());
//        Process::setUser($_SERVER['USER'], 'staff');
    }

	/**
	 *
	 * @memcheck
	 */
	public function testExecSimple() {
		$cmd = "sleep 0.1; echo 'stderr'>&2; echo 'stdout'";
		$this->promise(function () use ($cmd) {
			$res = yield Process::exec($cmd);
            $res->test_write_props = "hello";
			$this->data['instance'] = get_class($res);
			return (array)$res;
		});
		$this->loop();
		$this->assertEquals('ION\Process\ExecResult', $this->data['instance']);
		$this->assertEquals($cmd, $this->data['result']['command']);
		$this->assertTrue(is_integer($this->data['result']['pid']));
		$this->assertEquals('stdout', trim($this->data['result']['stdout']));
		$this->assertEquals('stderr', trim($this->data['result']['stderr']));
		$this->assertEquals(0, $this->data['result']['status']);
		$this->assertFalse($this->data['result']['signaled']);
		$this->assertEquals(0, $this->data['result']['signal']);

	}

	/**
	 * @memcheck
	 */
	public function testExecExtended() {
		$cmd = "sleep 0.1; echo \$_ION_EXEC_LINE;";
		$this->promise(function () use ($cmd) {
			$deferred = Process::exec($cmd, [
				'user' => 'www',
				'set_group' => true,
				'pid' => &$this->data['pid']
			]);
			$res = yield $deferred;
			$this->data['instance'] = get_class($res);
			return (array)$res;
		});
		$this->loop();
		$this->assertEquals('ION\Process\ExecResult', $this->data['instance']);
		$this->assertEquals($cmd, $this->data['result']['command']);
		$this->assertTrue(is_integer($this->data['result']['pid']));
		$this->assertTrue(is_integer($this->data['pid']));
		$this->assertEquals($this->data['result']['pid'], $this->data['pid']);
		$this->assertStringMatchesFormat(__FILE__.":%i", trim($this->data['result']['stdout']));
	}
}