<?php

$worker = new \ION\Process\Worker();

$worker->on("finish")->then($cb1);

$worker->run($cb2);