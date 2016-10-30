Документация
====

## Содержаение

* [Промисы](./promisor.md) (`ION\Promise`, `ION\Deferred`, `ION\Sequence`)
* [Иерерхия исключений](./exceptions.md)
* [Работа с файловой системой](./fs.md) (`ION\FS`)
    * [События файловой системы](./fs.md#События-файловой-системы)
    * [Чтение файлов](./fs.md#Чтение-файлов)
* [Работа и настройка DNS](./dns.md)
* [Работа с процессами](./process.md) (`ION\Process`, `ION\Process\ChildProcess`)
    * [Манипуляции с прцессом](./process.md#work)
    * [Сигналы](./process.md#signals) (`ION\Process\Signals`)
    * [Выполнение подпроцесса](./process.md#exec) (`ION\Process\Exec`)
    * [Дочерние процессы](./process.md#childs) (`ION\Process\ChildProcess`)
        * [Создание дочерних процессов](./process.md#create-childs)
        * [Межпроцесорные коммуникации](./process.md#ipc) (`ION\Process\IPC`, `ION\Process\IPC\Message`)
        * [Управление дочерними процессами](./process.md#childs)
* [Работа со слушающими сокетами](./listeners.md) (`ION\Listener`)
* [Работа с потоками](./streams.md) (`ION\Streams`)
    * [Создание потоков](./streams.md)
    * [Чтение из потока и запись в поток](./streams.md)
    * [События в потоке](./streams.md)
* [Парсеры](./parsers.md)
    * [HTTP парсер](./parsers.md#http) (`ION\HTTP\Request`, `ION\HTTP\Respose`)
    * [WebSocket парсер](./parsers.md#ws) (`ION\HTTP\WebSocket\Frame`)
    * [MultiPart парсер](./parsers.md#mp) (`ION\HTTP\MultiPart`, `ION\HTTP\MultiPart\Part`)