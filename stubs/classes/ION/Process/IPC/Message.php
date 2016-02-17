<?php

namespace ION\Process\IPC;


use ION\Process\Worker;

class Message {

    public function __construct(string $name) { }

    public function getWorker() : Worker {}

    public function getName() : string {}

    public function setData($data) : static {}

    public function getData() : mixed {}
}