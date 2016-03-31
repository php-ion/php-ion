<?php

namespace ION\HTTP\WebSocket;


use ION\Test\TestCase;

class FrameTest extends TestCase {

    public $mask = "mask";
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
     *
     * @memcheck
     */
    public function testBuild() {
        $frame = new Frame();
        $frame->withBody("frame data");
        $frame->withOpcode(Frame::OP_TEXT);
        $frame->withFinalFlag(true);
        $frame->withMasking($this->mask);
        $this->assertEquals($this->frame, $frame->build());
    }

    /**
     * @memcheck
     */
    public function testBuild200() {
        $frame = new Frame();
        $frame->withBody(str_pad("x", 200, "x"));
        $frame->withOpcode(Frame::OP_BINARY);
        $parsed = Frame::parse($frame->build());
        $this->assertEquals(str_pad("x", 200, "x"), $parsed->getBody());
        $this->assertEquals(Frame::OP_BINARY, $frame->getOpcode());
        $this->assertFalse($frame->getFinalFlag());
        $this->assertFalse($frame->hasMasking());
        $this->assertEquals("", $frame->getMasking());
    }

    /**
     * @memcheck
     */
    public function testBuild700000() {
        $frame = new Frame();
        $frame->withBody(str_pad("x", 700000, "x"));
        $frame->withOpcode(Frame::OP_BINARY);
        $frame->withMasking("mask");
        $parsed = Frame::parse($frame->build());
        $this->assertEquals(str_pad("x", 700000, "x"), $parsed->getBody());
        $this->assertEquals(Frame::OP_BINARY, $frame->getOpcode());
        $this->assertFalse($frame->getFinalFlag());
        $this->assertTrue($frame->hasMasking());
        $this->assertEquals("mask", $frame->getMasking());
    }
}