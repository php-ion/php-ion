<?php

/**
 * @author Ivan Shalganov <a.cobest@gmail.com>
 * @since 1.0
 * @static
 * */
class ION {
	const PRIORITY_LOW     = 5;
	const PRIORITY_DEFAULT = 3;
	const PRIORITY_HIGH    = 1;
	const PRIORITY_URGENT  = 0;
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
     * @todo remove flags
	 */
    public static function dispatch($flags = 0) {}

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
     * 
     * @param float $time time in seconds. Minimal value is 1e-6
     * @return \ION\Deferred
     */
    public static function await($time) {}

    /**
     * 
     * @param float $time
     * @param callable $callback
     * @param mixed $args
     * @return int
     */
    public static function setInterval($time, $callback, $args = null) {}

}
