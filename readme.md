ION Extension [dev]
===================

**ION** (regex `/^(I)nput(?:, |-)(O)utput,? (N)otifications$/i`) - PHP extension for asynchronous programming.

* **Subject:** PHP extension
* **Language:** C
* **OS:** linux, osx, freebsd
* **PHP version:** 7.0
* **State:** [![Build Status](https://travis-ci.org/php-ion/php-ion.png?branch=master)](https://travis-ci.org/php-ion/php-ion) [![Coverage Status](https://coveralls.io/repos/php-ion/php-ion/badge.svg?branch=master&service=github)](https://coveralls.io/github/php-ion/php-ion?branch=master)
* **Version:** [![Latest Stable Version](https://poser.pugx.org/phpion/phpion/v/stable)](https://packagist.org/packages/phpion/phpion) [![Latest Unstable Version](https://poser.pugx.org/phpion/phpion/v/unstable)](https://packagist.org/packages/phpion/phpion)
* **Versioning:** [semver2](http://semver.org/)
* **Based:** [libevent2](http://libevent.org/)
* **Packagist:** [phpion/phpion](https://packagist.org/packages/phpion/phpion)
* **Extension:** [classes](./stubs/classes), [ini](./stubs/ION.ini), [constants](./stubs/constants.php)
* **Testing system:** [phpunit](https://phpunit.de/) (+ memory leak detector)

### [Install](./docs/install.md) :: [Develop](./docs/develop.md) :: [Testing](./docs/testing.md) :: [Segfault](./docs/segfault.md)

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

# Indev

* Listening FS events
* SSL support

# Planned

* DNS server
* Server socket pool
* Client socket pool
* HTTP server
* HTTP client
* IPC


## Promisors

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
    $result[] = yield from App::eventualGenerator();
    // ...
    return $result;
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
$sequence = new Sequence(function ($data) {/* ... */});
$sequence->then()->then()->then(); // ...

$sequence("one"); // run sequence
$sequence("two"); // run sequence again
```

```php
App::eventualAction()->then(function () {
    // ...
    $result = yield App::someSequence(); // allow use sequence as promise
    // ...
});
```

### Working with promisors

```php
ION::promise(function () {

    // do eventual actions

})->then(/* ... */)
  ->then(/* ... */);
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
ION::interval(30)->then() // ... build sequence
```

```php
ION::interval(30, "crawler")->then() // ... build sequence
// ...
ION::cancelInterval("crawler");
```



## Streams

```php
use ION\Stream;
use ION\Promise;
use ION\Sequence;
```

```php
$stream = Stream::resource(STDIN);
list($stream1, $stream2) = Stream::pair();
$stream = Stream::connect("tcp://example.com:80");
$stream = Stream::connect("/var/run/server.sock");
$stream = Stream::connect("ssl://example.com:443");
```

```php
$data = $stream->get(1024);
$data = $stream->getLine("\r\n\r\n", 64 * KiB);
$data = $stream->getAll();

$data = yield $stream->read(1024);
$data = yield $stream->readLine("\r\n\r\n", 64 * KiB);
$data = yield $stream->readAll();

yield $stream->awaitConnection();
yield $stream->awaitShutdown();
yield $stream->flush();

$stream->onData()->then()->then() // ... build sequence
```

### Listeners

```php
use ION\Stream;
use ION\Listener;
use ION\Sequence;
```

```php
$listener = new Listener("tcp://0.0.0.0:8080");
$listener->connect()->then(function (Stream $connect) {
   // ...
}); // build sequence
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

### Streams storages

#### Server

```php
use ION\Stream;
use ION\Listener;
use ION\Stream\Server;
```

```php
$server  = new Server();
$server->listen("0.0.0.0:8080");
$server->listen("0.0.0.0:8443", $ssl);
$server->setIdleTimeout(30);
$server->setMaxConnections(1000);
$server->connect()->then(function (Stream $connect) {
    // ...
}) // ...

// ...

$stream = $server->getConnection("127.0.0.1:43762");
```

#### Client

```php
use ION\Stream;
use ION\Listener;
use ION\Stream\Server;
```

```php
$client  = new Client();
$client->socket("tcp://127.0.0.1:3306");
$client->socket("ssl://10.0.22.133:3312");
$client->setIdleTimeout(30);
$client->setMaxConnections(1000);
$client->onHandshake()->then($hanler1)->then($hanler2); // build sequence
$client->onShutdown()->then($hanler3)->then($hanler4); // build sequence

$stream = yield $client->getStream();
// ...
```

## DNS

```php
use ION\DNS;
use ION\Deferred;
```

```php
$info = yield DNS::resolve("example.com");
// [
//     'CNAME' => "example.com",
//     'A'     => ["93.184.216.34"],
//     'AAAA'  => ["2606:2800:220:1:248:1893:25c8:1946"]
// ];
```

## FS

```php
$data = yield FS::readFile($filename = "/path/to/file", $offset = 0, $limit = 1000);
```

### FS events

```php
FS::watch($path = "/path/to/file", $flags = FS::NOTIFY_CHANGE | FS::NOTIFY_RECURSIVE)->then($handler);
```

## Process

```php
use ION\Process;
```

```php
Process::setUser("www-data");
Process::setPriority(10);
```

### Signals

```php
use ION\Process;
use ION\Process\Signals;
```

```php
Process::kill(Signals::TERM, $pid); // send SIGTERM

Process::signal(Signals::TERM)->then($hanler); // sequence for signal SIGTERM
```

### Execute process

```php
use ION\Process;
```

```php
$result = yield Process::exec("venor/bin/phpunit --tap");
```

```php
$result = yield Process::exec("venor/bin/phpunit --tap", [
    "user" => "nobody",
    "group" => "nobody",
    "pid" => &$pid  // get the PID by reference
]);

var_dump($result->stdout);
var_dump($result->stderr);
```

### IPC

todo

## C API

todo
