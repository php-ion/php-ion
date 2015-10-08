<?php

namespace ION\Test;


use ION\Debug;

class DebugTest extends TestCase {

    public function providerFcallVoid() {
        return [
            [],
            [1],
            [1, 2],
            [1, 2, 3],
        ];
    }

    /**
     *
     * @memcheck
     * @dataProvider providerFcallVoid()
     * @param mixed $arg1
     * @param mixed $arg2
     * @param mixed $arg3
     */
    public function testFcallVoid($arg1 = null, $arg2 = null, $arg3 = null) {
        $callback = function () {
            return array_sum(func_get_args());
        };
        $res = 1;

        switch (func_num_args()) {
            case 0:
                $res = Debug::fcallVoid($callback);
                break;
            case 1:
                $res = Debug::fcallVoid($callback, $arg1);
                break;
            case 2:
                $res = Debug::fcallVoid($callback, $arg1, $arg2);
                break;
            case 3:
                $res = Debug::fcallVoid($callback, $arg1, $arg2, $arg3);
                break;
        }
        $this->assertEquals(0, $res);
    }

    /**
     *
     * @memcheck
     * @dataProvider providerFcallVoid()
     * @param mixed $arg1
     * @param mixed $arg2
     * @param mixed $arg3
     */
    public function testCbCallVoid($arg1 = null, $arg2 = null, $arg3 = null) {
        $callback = function () {
            return array_sum(func_get_args());
        };
        $res = 1;

        switch (func_num_args()) {
            case 0:
                $res = Debug::cbCallVoid($callback);
                break;
            case 1:
                $res = Debug::cbCallVoid($callback, $arg1);
                break;
            case 2:
                $res = Debug::cbCallVoid($callback, $arg1, $arg2);
                break;
            case 3:
                $res = Debug::cbCallVoid($callback, $arg1, $arg2, $arg3);
                break;
        }
        $this->assertEquals(0, $res);
    }


    /**
     * @group testCbCreate
     * @memcheck
     */
    public function testCbCreate() {
        Debug::globalCbCreate(function ($a) {
            $this->data['testCbCreate'] = $a;
            return "retval";
        });
        $this->assertEquals(0, Debug::globalCbCallVoid("test testCbCreateFromZval"));
        $this->assertEquals("test testCbCreateFromZval", $this->data['testCbCreate']);
    }

    /**
     * @group testCbCreateFromZval
     * @memcheck
     */
    public function testCbCreateFromZval() {
        Debug::globalCbCreateFromZval(function ($a) {
            $this->data['testCbCreateFromZval'] = $a;
            return "retval";
        });
        $this->assertEquals(0, Debug::globalCbCallVoid("test testCbCreateFromZval"));
        $this->assertEquals("test testCbCreateFromZval", $this->data['testCbCreateFromZval']);
    }

    public function example($a) {
        $this->data['testCbFetchMethod'] = $a;
        return "retval";
    }

    /**
     * @memcheck
     */
    public function testCbFetchMethod() {
        Debug::globalCbFetchMethod(__CLASS__, 'example');
        $obj = new ExampleObj();
        $this->assertEquals('retval', Debug::globalCbObjCall($obj, "test testCbCreateFromZval"));
        $this->assertEquals("test testCbCreateFromZval", $obj->data['testCbFetchMethod']);
    }

    /**
     * @memcheck
     */
    public function testCbCall() {
        Debug::globalCbCreate(function ($a) {
            $this->data['testCbCreate'] = $a;
            return "retval";
        });
        $this->assertEquals('retval', Debug::globalCbCall("test testCbCreateFromZval"));
        $this->assertEquals("test testCbCreateFromZval", $this->data['testCbCreate']);
    }
}

class ExampleObj extends DebugTest {

}