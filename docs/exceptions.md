Exceptions hierarchy
=====

* [ION\RuntimeException](#runtime) extends `RuntimeException`
    * [ION\PromiseException](#promise)
        * [ION\Promise\CancelException](#promise-cancel)
            * [ION\Promise\TimeoutException](#promise-timeout)
    * [ION\CryptoException](#crypto)
    * [ION\ListenerException](#listener)
    * [ION\StreamException](#stream)
    * [ION\FSException](#fs)
    * [ION\DNSException](#dns)
    * [ION\ProcessException](#process)
        * [ION\Process\SpawnException](#process-spawn)
    * [ION\Stream\StorageException](#storage)
* [ION\InvalidUsageException](#invalid-usage) extends `LogickException`