<?php

namespace ION\HTTP\Content\WebSocket;


class Frame {

    public $opcode   = 0;
    public $is_final = true;
    public $content;
}