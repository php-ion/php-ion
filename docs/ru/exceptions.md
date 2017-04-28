Иерархия исключений
=====

* `ION\RuntimeException` (расширяет `RuntimeException`)
    * `ION\PromiseException` 
        * `ION\Promise\CancelException`
            * `ION\Promise\TimeoutException`
    * `ION\CryptoException` — ошбики шифрования
    * `ION\ListenerException`
    * `ION\StreamException`
    * `ION\FSException`
    * `ION\DNSException`
    * `ION\ProcessException`
        * `ION\Process\SpawnException`
    * `ION\Stream\StorageException`
* `ION\InvalidUsageException` (расширяет `LogicException`) — ошибка не правильной настройки или использования элементов расширения