<?php

namespace ION;


use ION\Process\ChildProcess;

class Process {
    public static function stdin() : Stream { }
//    public static function setStdin($fd) { }

    public static function stdout() : Stream { }
//    public static function setStdout($fd) { }

    public static function stderr() : Stream { }
//    public static function setStderr($fd) { }

    /**
     * Create a child process and restore event loop.
     *
     * @param int $flags
     *
     * @return int On success, the PID of the child process is returned in the parent, and 0 is returned in the child.
     * @see man fork
     * @see \ION::reinit()
     */
    public static function fork($flags = 0) { }

    /**
     * Spawn new child process.
     *
     * @param int $flags
     * @param mixed $ctx
     *
     * @return int
     */
    public static function spawn(int $flags = 0, $ctx = null) : int {}

    /**
     * Set signal handler
     *
     * @param int $signo signal number or text name
     *
     * @return Sequence
     */
    public static function signal(int $signo) : Sequence { }

    /**
     * Remove signal(s) handlers
     *
     * @param int $signo if -1 remove all handlers from all signals
     */
    public static function clearSignal(int $signo = -1) { }

    /**
     * Send a signal to a process
     *
     * @param int $signo signal number
     * @param int $pid   process ID
     *
     * @return bool
     */
    public static function kill(int $signo, int $pid) : bool { }

    /**
     * Execute an external program
     *
     * @param string $command The command that will be executed.
     * @param array $options  execute options:
     *                        <pre>
     *                        [
     *                          'user'  => $user_name_or_uid,
     *                          'group' => $group_name_or_uid,
     *                          'cwd'   => $path,
     *                          'pid'   => &$pid
     *                        ]
     *                        </pre>
     *
     * @return Deferred
     * @see ION\Process\Exec
     * */
    public static function exec($command, array $options = []) : Deferred { }


    /**
     * Return the current process identifier
     * @return int
     */
    public static function getPid() : int { }

    /**
     * Return the parent process identifier
     * @return int
     */
    public static function getParentPid() : int { }

    /**
     * Get user information
     *
     * @param mixed $user integer means UID, string means user name, null or no argument means current user
     *
     * @return array|false information with keys: name, uid, gid, info, home, shell
     * @throws \RuntimeException if error occurs
     * @throws \InvalidArgumentException if user not found or invalid argument type
     */
    public static function getUser($user = null) { }

    /**
     * Get group information
     *
     * @param mixed $group integer means GID, string means group name, null or no argument means current user's group
     *
     * @return array|false information with keys: name, gid, and members (array of users)
     * @throws \RuntimeException if error occurs
     * @throws \InvalidArgumentException if user not found or invalid argument type
     */
    public static function getGroup($group = null) { }

    /**
     * Set process user
     *
     * @param mixed $user     integer means UID, string means username
     * @param bool $set_group set user's group too
     *
     * @return array previous user information
     * @throws \RuntimeException if error occurs
     * @throws \InvalidArgumentException if user not found or invalid argument type
     */
    public static function setUser($user, $set_group = true) : array { }

    /**
     * Get program scheduling priority
     *
     * @param int $pid
     *
     * @return int
     */
    public static function getPriority($pid = null) : int { }

    /**
     * Set program scheduling priority
     *
     * @param int $priority
     * @param int $pid
     *
     * @return int previous priority
     */
    public static function setPriority($priority, $pid = null) : int { }

    /**
     * @param int $pid
     *
     * @return bool
     */
    public static function hasChildProcess(int $pid) : bool { }

    /**
     * @param int $pid
     *
     * @return ChildProcess
     */
    public static function getChildProcess(int $pid) : ChildProcess { }

    /**
     * @return array
     */
    public static function getChildProcesses() : array { }
}