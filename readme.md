ION Extension [dev]
===================

**ION** (regexp `/^Input(?:, |-)Output,? Notifications$/`) - PHP extension for asynchronous IO and other notifications (such as POSIX signals, timers, inotify). Based on [libevent2](http://libevent.org/).

[![Build Status](https://travis-ci.org/php-ion/php-ion.png?branch=master)](https://travis-ci.org/php-ion/php-ion)


Features:
* Listen descriptor events
* Set timers
* Listen signals
* Handle socket
* Supports DNS requests
* Built-in DNS server
* Built-in HTTP server
* Supports sendfile
* Supports SSL
* Listen file system events via inotify
* Work with PHP resources well
* Resistant to fatal errors and exit functions
* Supports asynchronous yields
* Built-in connections pool
* Asynchronous execution of external programs
* Implementation basic process functions (fork, setgid, setuid ...)
* Have C API

[Thinking about asynchronous programming](docs/philosophy.md)