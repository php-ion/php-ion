<?php

namespace ION\Process;


class Signals {
    /**
     * Hangup
     */
    const SIGHUP = 1;
    /**
     * Interrupt
     */
    const SIGINT = 2;
    /**
     * Quit and dump core
     */
    const SIGQUIT = 3;
    /**
     * Illegal Instruction
     */
    const SIGILL = 4;
    /**
     * Trace/breakpoint trap
     */
    const SIGTRAP = 5;
    /**
     * Process aborted
     */
    const SIGABRT = 6;
    /**
     * Bus error: "access to undefined portion of memory object"
     */
    const SIGBUS = 7;
    /**
     * Floating point exception: "erroneous arithmetic operation"
     */
    const SIGFPE = 8;
    /**
     * Kill, terminate immediately. Cannot be caught, blocked or ignored.
     */
    const SIGKILL = 9;
    /**
     * User-defined signal #1
     */
    const SIGUSR1 = 10;
    /**
     * Segmentation violation
     */
    const SIGSEGV = 11;
    /**
     * User-defined signal #2
     */
    const SIGUSR2 = 12;
    /**
     * Write to pipe with no one reading
     */
    const SIGPIPE = 13;
    /**
     * Alarm Clock
     */
    const SIGALRM = 14;
    /**
     * Termination
     */
    const SIGTERM = 15;
    /**
     * Child stopped or terminated
     */
    const SIGCHLD = 16;
    /**
     * Continue if stopped
     */
    const SIGCONT = 17;
    /**
     * Stop process. Cannot be caught, blocked or ignored.
     */
    const SIGSTOP = 18;
    /**
     * Stop typed at tty
     */
    const SIGTSTP = 19;
    /**
     * tty input for background process
     */
    const SIGTTIN = 20;
    /**
     * tty output for background process
     */
    const SIGTTOU = 21;
    /**
     * Urgent data available on socket
     */
    const SIGURG = 22;
    /**
     * CPU time limit exceeded
     */
    const SIGXCPU = 23;
    /**
     * File size limit exceeded
     */
    const SIGXFSZ = 24;
    /**
     * Signal raised by timer counting virtual time: "virtual timer expired"
     */
    const SIGVTALRM = 25;
    /**
     * Profiling timer expired
     */
    const SIGPROF = 26;
    /**
     * Resize terminal
     */
    const SIGWINCH = 27;
    /**
     * Pollable event
     */
    const SIGPOLL = 28;
    /**
     * input-output event
     */
    const SIGIO = 28;
    /**
     * Power failure
     */
    const SIGPWR = 29;
    /**
     * Bad syscall
     */
    const SIGSYS = 30;

    public static function getSignalNameByNo($no) { }
}