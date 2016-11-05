Документация
====

## Содержаение

* [Установка](./install.md)
* [PHP API](./../../stubs/)
    * [Классы](./../../stubs/classes/)
    * [Константы](./../../stubs/constants.php)
    * [INI настройки](./../../stubs/ION.ini)
    * [Тесты](./../../tests/cases/)
    * Примеры
* [Промисы](./promisor.md) (`ION\Promise`, `ION\ResolvabePromise`, `ION\Deferred`, `ION\Sequence`)
* [Иерерхия исключений](./exceptions.md)
* [Работа с файловой системой](./fs.md) (`ION\FS`)
    * [События файловой системы](./fs.md#События-файловой-системы)
    * [Чтение файлов](./fs.md#Чтение-файлов)
* [DNS](./dns.md)
* [Работа с процессами](./process.md) (`ION\Process`, `ION\Process\ChildProcess`)
    * [Манипуляции с прцессом](./process.md#work)
    * [Сигналы](./process.md#signals) (`ION\Process\Signals`)
    * [Исполнение внешних программ](./process.md#Исполнение-внешних-программ) (`ION\Process\Exec`)
    * [Дочерние процессы](./process.md#Дочерние-процессы) (`ION\Process\ChildProcess`)
        * [Создание дочерних процессов](./process.md#create-childs)
        * [Межпроцесорные коммуникации](./process.md#ipc) (`ION\Process\IPC`, `ION\Process\IPC\Message`)
        * [Управление дочерними процессами](./process.md#childs)
* [Слушающие сокеты](./listeners.md) (`ION\Listener`)
* [Потоки](./streams.md) (`ION\Streams`)
    * [Создание потоков](./streams.md)
    * [Чтение из потока и запись в поток](./streams.md)
    * [События в потоке](./streams.md)
    * Фильтры
    * Групповые лимиты
* Хранилище потоков
    * Абстрактный сервер
    * Абстрактный клиент
* [HTTP](./http.md)
    * Парсеры
        * [HTTP](./http.md) (`ION\HTTP\Request`, `ION\HTTP\Respose`)
        * [WebSocket](./http.md) (`ION\HTTP\WebSocket\Frame`)
        * [MultiPart](./http.md) (`ION\HTTP\MultiPart`, `ION\HTTP\MultiPart\Part`)
    * HTTP сервер
    * HTTP клиент
* [C API](./capi.md)