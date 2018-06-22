ION PHP Extension
=================

**ION** (regex `/^(I)nput(?:,?\s|-)(O)utput,?\s(N)otifications?$/im`) - PHP extension for asynchronous programming.

* **Subject:** PHP extension
* **Language:** C
* **OS:** linux, mac, freebsd
* **PHP version:** 7.0+
* **Stage:** develop
* **State:** [![Build Status](https://travis-ci.org/php-ion/php-ion.png?branch=master)](https://travis-ci.org/php-ion/php-ion) [![Coverage Status](https://coveralls.io/repos/php-ion/php-ion/badge.svg?branch=master&service=github)](https://coveralls.io/github/php-ion/php-ion?branch=master)
* **Version:** [![Latest Stable Version](https://poser.pugx.org/phpion/phpion/v/stable)](https://packagist.org/packages/phpion/phpion) [![Latest Unstable Version](https://poser.pugx.org/phpion/phpion/v/unstable)](https://packagist.org/packages/phpion/phpion)
* **Versioning:** [semver2](http://semver.org/)
* **Based:** [libevent2](http://libevent.org/)
* **Packagist:** [phpion/phpion](https://packagist.org/packages/phpion/phpion)
* **Documentation:** en, [ru](./docs/ru/readme.md)
* **PHP API**: see [classes](./stubs/classes) and [constants](./stubs/constants.php)
* **Configuration**: see [ini](./stubs/ION.ini) directives
* **Unit testing:** [phpunit](https://phpunit.de/) with memory leak detector

### [Install](./docs/install.md) :: [Testing](./docs/testing.md) :: [Segfault](./docs/segfault.md) :: [Contributing](./.github/CONTRIBUTING.md##how-to-contribute-to-php-ion)

# Features

* Built-in Promise/Deferred/Sequence
* Any eventual action return Promise/Deferred/Sequence
* Any Promise/Deferred/Sequence supports generators
* Promise/Deferred/Sequence generators make asynchronous programming easy
* Promise/Deferred/Sequence supports type hinting in callbacks
* Async sockets and stream pipes
* Async socket listeners
* Sendfile supports
* Asynchronous DNS requests
* Useful utilities for processes
* Sending and listening POSIX signals
* Async execution an external program
* Async reading files from FS
* SSL/TLS encryption supports
* Listening FS events
* Create process childs
* Asynchronous inter-process communication
* Management of child processes
* Built-in HTTP 1.0 and 1.1 request/response parsers
* Built-in WebSocket frame parser and  Multi-parted parser

## [0.9]

[x] Supports PHP 7.2
[ ] Supports PHP 7.3
[ ] Add events classes: `ION\DescriptorEvent`, `ION\TimerEvent`, `ION\SignalEvent`, `ION\FS\INodeEvent`

## [1.0]

- [ ] Stable ION

# What can you do

* Asynchronous servers
* Asynchronous clients
* PHP daemons
* PHP clusters
* as you wish

# Documentation [EN](./docs/en/readme.md) [RU](./docs/ru/readme.md)
