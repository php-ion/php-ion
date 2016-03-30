<?php

namespace ION\HTTP\WebSocket;


class Frame {

    const OP_CONTINUE = 0x0;
    const OP_TEXT     = 0x1;
    const OP_BINARY   = 0x2;
    const OP_CLOSE    = 0x8;
    const OP_PING     = 0x9;
    const OP_PONG     = 0xA;

    const FLAG_OPCODE  = 0x0F;
    const FLAG_MASKING = 0x10;
    const FLAG_FINAL   = 0x20;

    public static function parse(string $frame) : static { }
    public static function factory(string $body, int $opcode = Frame::OP_TEXT, int $flags = Frame::FLAG_FINAL) : static { }

    public function getOpcode() : int { }
    public function withOpcode(int $opcode) : static { }

    public function getBody() : string {}
    public function withBody(string $body) : static {}

    public function getFinalFlag() : bool {}
    public function withFinalFlag(bool $fin_flag) : static {}

    public function hasMasking() : bool {}
    public function getMasking() : int {}
    public function withMasking(int $mask = null) : static {}
    public function withoutMasking() : static {}

    public function build() : string {}
}