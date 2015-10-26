ION Extension [dev]
===================

**ION** (preg `/^(I)nput(?:, |-)(O)utput,? (N)otifications$/i`) - PHP extension for asynchronous IO and other notifications (such as POSIX signals, timers, inotify).

* **Subject:** PHP extension
* **Language:** C
* **OS:** linux, osx
* **PHP version:** 7.0
* [![Build Status](https://travis-ci.org/php-ion/php-ion.png?branch=master)](https://travis-ci.org/php-ion/php-ion)
* [![Coverage Status](https://coveralls.io/repos/php-ion/php-ion/badge.svg?branch=master&service=github)](https://coveralls.io/github/php-ion/php-ion?branch=master)
* [![Latest Stable Version](https://poser.pugx.org/phpion/phpion/v/stable)](https://packagist.org/packages/phpion/phpion)
* [![Latest Unstable Version](https://poser.pugx.org/phpion/phpion/v/unstable)](https://packagist.org/packages/phpion/phpion)
* [![License](https://poser.pugx.org/phpion/phpion/license)](https://packagist.org/packages/phpion/phpion)
* **Versioning:** [semver](http://semver.org/)
* **Based:** [libevent2](http://libevent.org/)
* **Composer:** [phpion/phpion](https://packagist.org/packages/phpion/phpion)
* **Testing system:** [phpunit](https://phpunit.de/) (+ memory leak detector)

# Install

```
git clone https://github.com/php-ion/php-ion.git
cd php-ion
phpize
./configure --with-ion
make
make install
```

# Features

todo

## Promises

```php
use ION\Promise;
use ION\ResolvablePromise;
use ION\Deferred;
use ION\PromiseMap;
```

```php

App::someEventualAction()
    ->then(function () {})
    ->onFail(function() {})
    ->onDone(function() {})
    ->then(/* ... */)
    // ...

```

```php
App::someEventualAction()
    ->then(function ($x) {
        // ...
    }, function (Throwable $error)) {
        // ...
    }, function ($info) {
        // ...
    })

```



## Timers

```php
use ION;
use ION\Promise;
```

```php
ION::await(0.4)->then() // ...
```

```php
ION::interval(30)->then() // ...
```

## Process

todo

### Signals

todo

### Execute process

todo

## Streams

todo

### Listeners

todo

### Groups

todo

### Connection pool

todo

### SSL

todo

## DNS

todo

### DNS responder

todo

## FS

todo

### FS events

todo

## Data structures

todo

## Testing

todo

## C API

todo
