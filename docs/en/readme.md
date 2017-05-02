Coming soooooon
===

## Below not formatted docs


# Promisor

## Concepts

### Deferred

A Deferred represents a computation or unit of work that may not have completed yet.
Typically (but not always), that computation will be something that executes asynchronously and completes at some point in the future.

### Promise

While a deferred represents the computation itself, a Promise represents the result of that computation.
Thus, each deferred is a promise that acts as a placeholder for actual result.

### Sequence

For recurrent events requires already created a promise chain. Sequence is promise, which may be invoked many times.

## API

If you've never heard about promises before, [read this first](https://gist.github.com/domenic/3889970).

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
    })

```


### Resolution forwarding

Resolved promises forward resolution values to the next promise.

Each call to then() returns a new promise that will resolve with the return value of the previous handler.
This creates a promise "pipeline".

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

### Rejection forwarding

Rejected promises behave similarly, and also work similarly to try/catch:
When you catch an exception, you must rethrow for it to propagate.

```php
App::someEventualAction()
    ->then(function () {
        throw new \RuntimeException("error occured", 5);
    })
    ->then(null, function (\Exception $e) {
        return $e->getCode();
    })
    ->then(function ($code) {
        echo "Code " . $code; // $code == 5
    })
```

### Cooperative multitasking

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
ION\FS::watch('config.php')->then(function () {
    // reconfigure application every time then the config file changed
});
```

```php
App::eventualAction()->then(function () {
    // ...
    $result = yield ION\FS::watch('config.php'); // use sequence as promise (one-shot event)
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
$stream = Stream::socket("example.com:80");
$stream = Stream::socket("/var/run/server.sock");
$stream = Stream::socket("example.com:443", Crypto::client());
$stream = Stream::socket(["93.184.216.34:0" => "example.com:443"]); // connecting from IP 93.184.216.34 to example.com:443
```

```php
$data = $stream->get(1024);
$data = $stream->getLine("\r\n\r\n", 64 * KiB);
$data = $stream->getAll();

$data = yield $stream->read(1024);
$data = yield $stream->readLine("\r\n\r\n", 64 * KiB);
$data = yield $stream->readAll();

yield $stream->connect();
yield $stream->awaitShutdown();
yield $stream->flush();

$stream->incoming()->then(...$handlers)->then(...$handlers);
```

### Listeners

```php
use ION\Stream;
use ION\Listener;
use ION\Sequence;
```

```php
$listener = new Listener("0.0.0.0:8080");
$listener->accept()->then(function (Stream $connect) {
   // ...
}); // build sequence
```

### SSL/TLS encryption

```php
use ION\Crypto;
use ION\Stream;
```

```php
$crypto = Crypto::client()->allowSelfSigned();
$stream = Stream::socket("example.com:443", $crypto);
// ...
```

```php
$crypto = Crypto::server(SSL::METHOD_TLSv12)->loadCert('cacert.pem', 'cakey.pem')->allowSelfSigned();
$listener = new Listener("0.0.0.0:8080");
$listener->encrypt($crypto);
// ...
```

### Streams storage

#### Server

```php
use ION\Stream;
use ION\Listener;
use ION\Stream\Server;
```

```php
$server  = new Server();
$server->listen("0.0.0.0:8080");
$server->listen("0.0.0.0:8443")->encrypt($ssl);
$server->setIdleTimeout(30);
$server->setMaxPoolSize(1000);
$server->accept()->then(function (Stream $connect) {
    // ...
}) // ...

// ...

$stream = $server->getStream("127.0.0.1:43762");
```

#### Client

```php
use ION\Stream;
use ION\Stream\Client;
```

```php
$client = new Client();
$client->socket("127.0.0.1:3306");
$client->socket("10.0.22.133:3312", $ssl);
$client->setIdleTimeout(30);
$client->setMaxConnections(1000);
$client->handshake()->then($handler1); // sequence
$client->onShutdown()->then($handler2); // sequence

$stream = yield $client->connect();
// ...
```

## DNS

```php
use ION\DNS;
use ION\Deferred;
```

```php
$info = yield DNS::resolve("example.com");
/* @var array $info */
// [
//     'CNAME' => "example.com",
//     'A'     => ["93.184.216.34"],
//     'AAAA'  => ["2606:2800:220:1:248:1893:25c8:1946"]
// ];
```

## FS

```php
$data = yield FS::readFile($filename = "/path/to/file", $offset = 0, $limit = 1000);
/* @var string $data */
```

### FS events

```php
FS::watch("/path/to/file")->then($handler);
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
$result = yield Process::exec("vendor/bin/phpunit --tap");
/* @var ION\Process\ExecResult $result */
```

```php
$result = yield Process::exec("vendor/bin/phpunit --tap", [
    "cwd"   => "/data/project",
    "user"  => "nobody",
    "group" => "nobody",
    "pid"   => &$pid  // get the PID by reference
]);
/* @var ION\Process\Exec $result */
var_dump($result->stdout);
var_dump($result->stderr);
```

### Multi-Process management

```php
use ION\Process;
use ION\Process\ChildProcess;
use ION\Process\IPC;
use ION\Process\IPC\Message;
```

Creates new child process

```php
$child = new ChildProcess();

$child->whenStarted()->then(function (ChildProcess $process) {
    // notify parent process when child has ben started after fork
}); 

$child->whenExited()->then(function (ChildProcess $process) {
    // notify parent process when child has ben exit
}); 

$child->start(function (IPC $ipc) {
    // this callback runs in the child process
    // $ipc - link with parent process
});

```

Inter-processes communication

```php
$child = Process:getChildProcess($pid);

// send data to child process
$child->getIPC()->send(json_ecnode($data));

// receive data from child process
$child->getIPC()->whenMessage(function (Message $message) {
    // $message->data - data from child process
    // $message->ctx - child process object
});

```


## HTTP

### HTTP Client

```php
$request = ION\HTTP\Request::factory()
    ->withMethod("GET")
    ->withUri(ION\URI::parse("http://example.com/?iddqd=on"))
    ->withHeader("user-agent", "ION HTTP Client Example");

$respose = yield ION\HTTP::request($request);
/* @var ION\HTTP\Response $respose */
$headers = $response->getHeaders();
$body = yield $response->readBody();
```

### HTTP Server

todo

### Workers and IPC

```php
use ION\Process;
use ION\Process\Worker;
use ION\Process\Message;
```

How to create worker:

```php

$worker = Process::createWorker();
// $worker instanceof Worker

$worker->run(function (Worker $worker) {
    // worker's code there
});

$worker->onExit()->then(function (Worker $worker) {
    // callback invokes then worker exited (success, with error or just killed)
});

```

Worker will be create delayed, when dispatcher continues work.

How to send message from worker or to worker:

```php
$worker = Process::createWorker();

$worker->onMessage()->then(function (Message $message) {
    $data = $message->getData();
});

$worker->run(function (Worker $w) {
    $w->send("some message");
});

```