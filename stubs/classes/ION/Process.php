<?php

namespace ION;


class Process {
	/**
	 * Create a child process and restore event loop.
	 *
	 * @param int $flags
	 * @param Stream[] $ipc
	 * @return int On success, the PID of the child process is returned in the parent, and 0 is returned in the child.
	 * @see man fork
	 * @see \ION::reinit()
	 */
	public static function fork($flags = 0, &$ipc = null) {}

	/**
	 * Set signal handler
	 * @param int|string $signo signal number or text name
	 * */
	public static function onSignal($signo, callable $cb) {}

	/**
	 * Set signal handler
	 * @param int|string $signo signal number or text name
	 * @return Deferred
	 * */
	public static function awaitSignal($signo) {}

	/**
	 * Send a signal to a process
	 * @param int $signo signal number
	 * @param int $pid process ID
	 * */
	public static function signal($signo, $pid) {}

	/**
	 * Execute an external program
	 *
	 * @param string $command The command that will be executed.
	 * @param array $options execute options:
	 * <pre>
	 * [
	 *   'env'   => [ $name => $value, ...],
	 *   'user'  => $user_name_or_uid,
	 *   'group' => $group_name_or_uid,
	 *   'priority' => 0,
	 *   'max_size' => 10 * MiB,
	 *   'pid'   => &$pid
	 * ]
	 * </pre>
	 * @return Deferred
	 * */
	public static function exec($command,  array $options = array()) {}


	/**
	 * Return the current process identifier
	 * @return int
	 * */
	public static function getPid() {}

	/**
	 * Return the parent process identifier
	 * @return int
	 * */
	public static function getParentPid() {}

	/**
	 * Get user information
	 * @param mixed $user, integer means UID, string means username, null or no argument means current user
	 * @return array information with keys: name, uid, gid, info, home, shell
	 * @throws \RuntimeException if error occurs
	 * @throws \InvalidArgumentException if user not found or invalid argument type
	 */
	public static function getUser($user = null) {}

	/**
	 * Set process user
	 * @param mixed $user integer means UID, string means username
	 * @param bool $set_group set user's group too
	 * @return array previous user information
	 * @throws \RuntimeException if error occurs
	 * @throws \InvalidArgumentException if user not found or invalid argument type
	 */
	public static function setUser($user, $set_group = true) {}

	/**
	 * Get program scheduling priority
	 * @param int $pid
	 * @return int
	 */
	public static function getPriority($pid = null) {}

	/**
	 * Set program scheduling priority
	 * @param int $priority
	 * @param int $pid
	 * @return int previous priority
	 */
	public static function setPriority($priority, $pid = null) {}
}