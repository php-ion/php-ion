<?php

namespace ION\HTTP\WebSocket;


use ION\Test\TestCase;

class FrameTest extends TestCase {

    public $mask = 4584902216;
    public $frame = "\x81\x8Amask\x0B\x13\x12\x06\x08\x41\x17\x0A\x19\x00";

    /**
     * @memcheck
     */
    public function testParse() {
        $frame = Frame::parse($this->frame);
        $this->assertEquals("frame data", $frame->getBody());
        $this->assertEquals(Frame::OP_TEXT, $frame->getOpcode());
        $this->assertTrue($frame->getFinalFlag());
        $this->assertTrue($frame->hasMasking());
        $this->assertEquals($this->mask, $frame->getMasking());
    }

    /**
     * @group dev
     * @memcheck
     */
    public function _testBuild() {
        $frame = new Frame();
        $frame->withBody("frame data");
        $frame->withOpcode(Frame::OP_TEXT);
        $frame->withFinalFlag(true);
        $frame->withMasking($this->mask);
        $this->assertEquals($this->frame, $frame->build());
    }
}