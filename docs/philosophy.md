Philosophy
==========


Typical syntax
--------------

Use callback

```php
<?php
    function http_get_request($url, $callback, $arg) {
        // open socket
        Stream::connect($host)->onConnect(function ($socket, $error) use ($callback, $arg) {
            if($error) {
                call_user_func($callback, null, $error, $arg);
            }
            // read headers
            $socket->gets("\r\n\r\n", function ($data, $error) use ($callback, $arg, $socket) {
                if($error) {
                    call_user_func($callback, null, $error, $arg);
                    $socket->close();
                }
                $headers = http_parse_headers($data);
                // read body
                http_read_body($socket, $headers, function ($body, $error) use ($callback, $arg, $socket, $headers) {
                    if($error) {
                        call_user_func($callback, null, $error, $arg);
                        $socket->close();
                    }
                    // 'return' result
                    call_user_func($callback, ["headers" => $headers, "body" => $body], $error, $arg);
                });
            });
        });
    }

    http_get_request($url, function ($data, $error, $scope) {
        if($error) {
            echo "Error: ".$error->getMessage();
        } else {
            echo "Got a page: ".$data["body"];
        }
    }, $scope);
?>
```

* many code
* different call scope
* 4 callbacks created and invoked
* difficult to read the code
* processing results in callbacks (another scope)

Yield syntax
------

Use `yield`.

```php
<?php

    function http_get_request($url) {
        $socket = ( yield Stream::connect($host) );
        $headers = http_parse_headers( yield $socket->gets("\r\n\r\n") );
        $body = ( yield http_read_body($socket, $headers) );
        return ["headers" => $headers, "body" => $body]
    }

    try {
        $data = ( yield http_get_request($url) );
    } catch(Exception $e) {
        Log("Error occurred", $e);
        // ...
    }
?>
```

* it's habitually
* it's simple
* it's fast
* it's clear
* it's nice

**Problem: `http_get_request` return generator, but generator cannot return any values**
Possible solutions:
* last simple value from `yield` may be result (store last yield-value).
* patch php and create Pull Request: `Generator::getReturnedValue();` or `Generator::$returned;`, destroy zval in generator's dtor