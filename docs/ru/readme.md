Документация
====

## Содержаение

* [Установка](./install.md)
* [PHP API](./../../stubs/)
    * [Классы](./../../stubs/classes/)
    * [Константы](./../../stubs/constants.php)
    * [INI настройки](./../../stubs/ION.ini)
    * [Тесты](./../../tests/cases/)
* [Промисы](./promisor.md) (`ION\Promise`, `ION\ResolvabePromise`, `ION\Deferred`, `ION\Sequence`)
    * [Promise](./promisor.md#promise)
    * [Deferred](./promisor.md#deferred)
    * [Sequence](./promisor.md#sequence)
    * [Цепочки](./promisor.md#Цепочки)
    * [Корутины](./promisor.md#Корутины)
* [Иерерхия исключений](./exceptions.md)
* [Работа с файловой системой](./fs.md) (`ION\FS`)
    * [События файловой системы](./fs.md#События-файловой-системы)
    * [Чтение файлов](./fs.md#Чтение-файлов)
* [DNS](./dns.md) (`ION\DNS`)
* [Работа с процессами](./process.md) (`ION\Process`, `ION\Process\ChildProcess`)
    * [Манипуляции с прцессом](./process.md#work)
    * [Сигналы](./process.md#signals) (`ION\Process\Signals`)
    * [Исполнение внешних программ](./process.md#Исполнение-внешних-программ) (`ION\Process\Exec`)
    * [Дочерние процессы](./process.md#Дочерние-процессы) (`ION\Process\ChildProcess`)
        * [Создание дочерних процессов](./process.md#Создание-дочерних-процессов)
        * [Межпроцесорные коммуникации](./process.md#Межпроцессное-взаимодействие) (`ION\Process\IPC`, `ION\Process\IPC\Message`)
        * [Управление дочерними процессами](./process.md#childs)
* [Шифрование SSL](./crypto.md) (`ION\Crypto`)
* [Слушающие сокеты](./listener.md) (`ION\Listener`)
    * [Шифрование](./listener.md#Шифрование)
* [Потоки](./streams.md) (`ION\Streams`)
    * [Создание потоков](./streams.md)
    * [Чтение из потока и запись в поток](./streams.md)
    * [События в потоке](./streams.md)
    * [Шифрование](./streams.md)
    * Фильтры
    * Групповые лимиты
* [HTTP](./http.md)
    * Парсеры
        * [URI](./http.md) (`ION\URI`)
        * [HTTP](./http.md) (`ION\HTTP\Request`, `ION\HTTP\Respose`)
        * [WebSocket](./http.md) (`ION\HTTP\WebSocket\Frame`)
        * [MultiPart](./http.md) (`ION\HTTP\MultiPart`, `ION\HTTP\MultiPart\Part`)
* Примеры
    * HTTP-прокси сервер
    * WebSocket чат 
* C API