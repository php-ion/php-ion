<?php

namespace ION\HTTP\WebSocket;


class Frame {

    const OP_CONTINUE = 0x0;
    const OP_TEXT     = 0x1;
    const OP_BINARY   = 0x2;
    const OP_CLOSE    = 0x8;
    const OP_PING     = 0x9;
    const OP_PONG     = 0xA;

    const OP_FLAGS   = 0x0F;
    const MASKED     = 0x10;
    const FIN        = 0x20;

    public static function parse(string $frame) : static { }

    public function getOpcode() : int { }
    public function withOpcode(int $opcode) : static { }

    public function getBody() : string {}
    public function withBody(string $body) {}

    public function getFinalFlag() : bool {}
    public function withFinalFlag(bool $fin_flag) {}

    public function build() : string {}
}