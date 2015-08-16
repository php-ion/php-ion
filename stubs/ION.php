<?php

/**
 * @author Ivan Shalganov <a.cobest@gmail.com>
 * @since 1.0
 * @static
 * */
class ION {
    /**
     * @since 1.0
     * Largest number of priorities that Libevent can support.
     * */
    const MAX_PRIORITIES = 256;

    /**
     * @since 1.0
     * Libevent version
     * */
    const LIBEVENT_VERSION = '2.0.16';

    /**
     * @since 1.0
     * Libevent version as hexadecimal integer
     * */
    const LIBEVENT_VERSION_INT = 0x020016;

    /**
     * Version of ION
     * */
    const VERSION = '1.0';

    /**
     * Version of ION as integer
     * */
    const VERSION_INT = 100;

    /**
     * Block until we have an active event, then exit once all active events have had their callbacks run.
     * */
    const ONCE = 1;

    /**
     * Do not block: see which events are ready now, run the callbacks of the highest-priority ones, then exit.
     * */
    const NONBLOCK = 2;
    const READ = 1;
    const WRITE = 2;
    const PERSIST = 4;
    const EV_ET = 8;
    
    /**
     * Reinit timer events
     */
    const REINIT_TIMERS = 1;
    /**
     * Reinit signal events
     */
    const REINIT_SIGNALS = 2;

    /**
     * Reinitialize the event dispatcher after a fork.
     * Some event mechanisms do not survive across fork. The event base needs to be reinitialized with this method.
     * 
     * @since 1.0
     * @param int $flags 
     * @throws \RuntimeException if error occurs
     * */
    public static function reinit($flags = 0) {}

	/**
	 * Run event dispatcher.
	 * Wait for events to become active, and run their callbacks.
	 * By default, this loop will run the event dispatcher until either there are no more added events,
	 * or until something calls ION::stop().
	 *
	 * @since 1.0
	 * @param int $flags any combination of ION::ONCE, ION::NONBLOCK
	 * @param int $timeout
     * @todo remove flags
	 */
    public static function dispatch($flags = 0, $timeout = 0) {}

    /**
     * Exit the event dispatcher after the specified time.
     * The next dispatcher iteration after the given timer expires will complete normally (handling all queued events)
     * then exit without blocking for events again.
     *
     * @since 1.0
     * @param int $timeout the amount of time after which the loop should terminate, or NULL to exit after running all currently active events.
     * @throws \RuntimeException if error occurs
     * */
    public static function stop($timeout = -1) {}

    /**
     * Set the number of different event priorities.
     *
     * By default Libevent schedules all active events with the same priority. However, some time it is desirable to process some
     * events with a higher priority than others. For that reason, Libevent supports strict priority queues.
     * Active events with a lower priority are always processed before events with a higher priority.
     *
     * The number of different priorities can be set initially with this method.
     * This function should be called before the first call to ION::dispatch().
     * By default, Libevent assigns the middle priority to all events unless their priority is explicitly set.
     *
     * Note that urgent-priority events can starve less-urgent events: after running all urgent-priority callbacks,
     * Libevent checks for more urgent events again, before running less-urgent events.
     * Less-urgent events will not have their callbacks run until there are no events more urgent than them that want to be active.
     *
     * @since 1.0
     * @param int $npriorities the maximum number of priorities (no more then ION::MAX_PRIORITIES)
     * @throws \RuntimeException if error occurs
     * @throws \InvalidArgumentException if passed invalid value
     * */
    public static function setMaxPriorities($npriorities) {}

    /**
     * 
     * @param float $time time in seconds. Minimal value is 1e-6
     * @param callable $callback
     * @param mixed $args
     * @return int
     */
    public static function setTimeout($time, $callback, $args = null) {}

    /**
     * 
     * @param float $time
     * @param callable $callback
     * @param mixed $args
     * @return int
     */
    public static function setInterval($time, $callback, $args = null) {}
    
    /**
     * 
     * @param callable $cb
     * @param array $arg
     */
    public static function deferred(callable $cb, array $args = array()) {}
    
    /**
     * Return current DNS server
     * @return ION\DNS
     */
    public static function getDNS() {}


}
