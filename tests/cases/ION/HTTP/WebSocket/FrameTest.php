<?php

namespace ION\HTTP\WebSocket;


use ION\Test\TestCase;

class FrameTest extends TestCase {

    /**
     * @group dev
     * @memcheck
     */
    public function testParse() {
        $raw = "\x81\x8Amask\x0B\x13\x12\x06\x08\x41\x17\x0A\x19\x00";
        $frame = Frame::parse($raw);
        $this->assertEquals("frame data", $frame->body);
        $this->assertEquals(Frame::OP_TEXT | Frame::FIN | Frame::MASKED, $frame->flags);
    }
}