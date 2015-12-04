<?php

use ION\Sequence;
use ION\Deferred;
use ION\Promise;
use ION\RuntimeException;

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
    const VERSION = 'x.y-z';

    /**
     * Block until we have an active event, then exit once all active events have had their callbacks run.
     * */
    const LOOP_ONCE = 1;

    /**
     * Do not block: see which events are ready now, run the callbacks of the highest-priority ones, then exit.
     * */
    const LOOP_NONBLOCK = 2;

    /**
     * Indicates that a timeout has occurred.
     */
    const EV_TIMEOUT = 0x01;
    /**
     * Wait for a socket or FD to become readable
     */
    const EV_READ = 0x02;
    /**
     * Wait for a socket or FD to become writeable
     */
    const EV_WRITE = 0x04;
    /**
     * Wait for a POSIX signal to be raised
     */
    const EV_SIGNAL = 0x08;
    /**
     * Persistent event
     */
    const EV_PERSIST = 0x10;
//	const EV_ET = 0x20;

    const DEBUG = 0;

    /**
     * Reinitialize the event dispatcher after a fork.
     * Some event mechanisms do not survive across fork. The event base needs to be reinitialized with this method.
     *
     * @since 1.0
     * @param int $flags
     * @return bool
     * @throws \RuntimeException if error occurs
     * */
    public static function reinit($flags = 0) : bool {}

    /**
     * Run event dispatcher.
     * Wait for events to become active, and run their callbacks.
     * By default, this loop will run the event dispatcher until either there are no more added events,
     * or until something calls ION::stop().
     *
     * @since 1.0
     * @param int $flags any combination of ION::ONCE, ION::NONBLOCK
     * @throws RuntimeException if error occurs
     * @return bool
     */
    public static function dispatch(int $flags = 0) : bool {}

    /**
     * Stop the event dispatcher after the specified time.
     * The next dispatcher iteration after the given timer expires will complete normally (handling all queued events)
     * then exit without blocking for events again.
     *
     * @since 1.0
     * @param float $timeout the amount of time after which the dispatch should terminate.
     * @throws RuntimeException if error occurs
     * */
    public static function stop(float $timeout = 0.0) {}

    /**
     * Asynchronous sleep.
     * Resolve promisor after
     *
     * @see interval
     * @since 1.0
     * @param float $time time in seconds. Minimal value is 1e-6
     * @return Deferred
     */
    public static function await(float $time) : Deferred {}

    /**
     * @see await
     * @see cancelInterval
     * @since 1.0
     * @param float $time time in seconds. Minimal value is 1e-6
     * @param string $name interval name
     * @return Sequence
     * @throws RuntimeException if error occurs
     */
    public static function interval(float $time, string $name = null) : Sequence {}

    /**
     * Stop named interval
     *
     * @see interval
     * @since 1.0
     * @param string $name
     * @return bool
     */
    public static function cancelInterval(string $name) {}

    /**
     * To promise anything
     * @param mixed $resolver
     * @return Promise
     */
    public static function promise($resolver) {}

}
