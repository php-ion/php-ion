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
* **Versioning:** [semver 2.0](http://semver.org/)
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

## Promisor

```php
use ION\Promise;
use ION\ResolvablePromise;
use ION\Deferred;
use ION\Sequence;
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


### Results routing

```php
App::someEventualAction()
    ->then(function () {
        return [1,2,3];
    })
    ->then(function (int $id) {
        // skip
    })
    ->then(function (array $ids) {
        // invoke
    })
```

### Generators

```php
App::someEventualAction()->then(function ($data) {
    // ...
    $result[] = yield App::eventualAction1();
    // ...
    $result[] = yield App::eventualAction2();
    // ...
    $result[] = yield App::eventualAction3();
    // ...
});
```

```php
App::someEventualAction()->then(function ($data) {
    // ...
    $items = yield App::anotherEventualAction($data, true);
    // ...
    if(yield App::awaitAction()) {
        // ...
    }
    // ...
    try {
        $info = yield App::asyncLoadInfo($data);
    } catch(ION\Promise\CancelException $cancel) {
        return false;
    } catch (Trowable $error) {
        yield App::rejectEventualAction($data);
        throw $error;
    }
    // ...
    return $info;
})
```

### Sequence

```php
$sequence = new Sequence(function ($data) {
    // ...
});
$sequence->then()->then()->then(); // ...

$sequence("one"); // run sequence
$sequence("two"); // run new sequence again
```

## Timers

```php
use ION;
use ION\Promise;
use ION\Sequence;
```

```php
ION::await(0.4)->then() // ...
```

```php
ION::startInterval(30)->then() // ...
```

```php
ION::startInterval(30, "crawler")->then() // ...
// ...
ION::stopInterval("crawler");
```



## Streams

```
use ION\Stream;
```

```php

$stream = Stream::resource(STDIN);
list($stream1, $stream2) = Stream::pair();
$stream = Stream::connect("tcp://google.com");
$stream = Stream::connect("/var/run/redis.sock");
$stream = Stream::connect("ssl://google.com");

```

### Listeners

```
use ION\Stream;
use ION\Listener;
```

```php

$listener = new Listener("tcp://0.0.0.0:8080");
$listener->onConnect(function (Stream $connect) {
    // ...
})->then()->then(); // ...
```

### Groups

```php
use ION\Stream;
use ION\Listener;
use ION\Stream\Group;
```

```php
$group = new Group();
$group->setRateLimit(/* ... */);
// ...
$stream->setGroup($group);
```

### Connection pool

```php
use ION\Stream;
use ION\Listener;
use ION\Stream\Server;
```

```php
$server  = new Server();
$server->listen("tcp://0.0.0.0:8080");
$group->setRateLimit(/* ... */);
$listener->onConnect(function (Stream $connect) {
    // ...
})->then()->then(); // ...

// ...

$stream = $server->getConnection("127.0.0.1:43762");
```

## DNS

```php
use ION\DNS;
```

```php
$info = yield DNS::resolve("google.com");
```

### DNS responder

todo

## FS

todo

### FS events

todo

## Process

todo

### Signals

todo

### Execute process

todo

## Testing

```
composer install
```

```
vendor/bin/phpunit
```

```
php -dextension=/path/to/ion.so vendor/bin/phpunit
```

## C API

todo
