<?php

namespace ION\HTTP;

use ION\HTTP\WebSocket\Frame;
use ION\Stream;
use ION\Test\TestCase;

/**
 * Class WebSocketParserTest
 * @package ION\HTTP
 */
class WebSocketParserTest extends TestCase {

    public $mask = "mask";
    public $frame = "\x81\x8Amask\x0B\x13\x12\x06\x08\x41\x17\x0A\x19\x00";
    public $body = "frame data";

    /**
     * @memcheck
     */
    public function testFrame() {
        $parser = new WebSocketParser();
        $parser->frame()->then(function (Frame $frame) {
            $this->data[] = $frame;
        });
        $parser($this->frame);

        $this->assertCount(1, $this->data);
        $this->assertEquals("frame data", $this->data[0]->getBody());
        $this->assertEquals(Frame::OP_TEXT, $this->data[0]->getOpcode());
        $this->assertTrue($this->data[0]->getFinalFlag());
        $this->assertTrue($this->data[0]->hasMasking());
        $this->assertEquals($this->mask, $this->data[0]->getMasking());
    }

    /**
     * @memcheck
     */
    public function testFrames() {
        $parser = new WebSocketParser();
        $parser->frame()->then(function (Frame $frame) {
            $this->data[] = $frame;
        });
        $parser($this->frame.$this->frame);
        $parser($this->frame);

        $this->assertCount(3, $this->data);
        for($i=0; $i<3; $i++) {
            $this->assertEquals("frame data", $this->data[$i]->getBody());
            $this->assertEquals(Frame::OP_TEXT, $this->data[$i]->getOpcode());
            $this->assertTrue($this->data[$i]->getFinalFlag());
            $this->assertTrue($this->data[$i]->hasMasking());
            $this->assertEquals($this->mask, $this->data[$i]->getMasking());
        }
    }

    public function providerFrameParts() {
        $steps = [];
        $frame = $this->frame.$this->frame;
        $len = strlen($frame);
        for($i = 1; $i < $len; $i++) {
            $steps[] = array_map('base64_encode', str_split($frame, $i));
        }
        return $steps;
    }

    /**
     * @group dev
     *
     * @param string[] ...$chunks
     *
     * @dataProvider providerFrameParts
     */
    public function testChunked(...$chunks) {
        $chunks = array_map('base64_decode', $chunks);
        $parser = new WebSocketParser();
        $parser->frame()->then(function (Frame $frame) {
            $this->data[] = $frame;
        });
        for($i = 0; $i < count($chunks); $i++) {
            $parser($chunks[$i]);
        }
        $this->assertCount(2, $this->data);
        foreach($this->data as $frame) {
            /* @var Frame $frame */
            $this->assertEquals("frame data", $frame->getBody());
            $this->assertEquals(Frame::OP_TEXT, $frame->getOpcode());
            $this->assertTrue($frame->getFinalFlag());
            $this->assertTrue($frame->hasMasking());
            $this->assertEquals($this->mask, $frame->getMasking());
        }
    }
}