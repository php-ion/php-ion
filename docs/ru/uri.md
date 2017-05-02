URI
===

`use ION\URI;`

Паресер строк в формате URI.
 
Работает аналогично встроенной функции [parse_url](http://php.net/manual/en/function.parse-url.php) только в виде объекта, 
который реализовывает [Psr\Http\Message\UriInterface](http://www.php-fig.org/psr/psr-7/#psrhttpmessageuriinterface) из рекомендаций [PSR-7](http://www.php-fig.org/psr/psr-7/) с небольшими изменениями.

```php

$uri = URI("http://root:123@example.com:8080/fake/index.html?param=val#anchor");

$uri->getScheme() === "http"; // true
$uri->withScheme("https")->getScheme() == "https"; // true

```

Помимо модифицирующих методов `with*` URI-объект можно создать через универсальную фабрику:

```php
$uri = URI::factory([
    URI::SCHEME => "https",
    URI::HOST => "example.com",
    URI::PORT => 443,
    URI::PATH => /robots.txt
]); 
```