<?php
define('ION_RESOURCES', __DIR__ . "/resources");
define('ION_VAR', __DIR__ . "/var");

//require_once __DIR__ . "/TestCase.php";
//require_once __DIR__ . "/TestCase/Server.php";

ini_set('date.timezone', 'Europe/Moscow');

if (!defined('ION_TEST_SERVER_HOST')) {
    define('ION_TEST_SERVER_HOST', "127.0.0.1:8976");
}
if (!defined('ION_TEST_SERVER_IPV4')) {
    define('ION_TEST_SERVER_IPV4', "127.0.0.1:8976");
}
if (!defined('ION_TEST_SERVER_IPV6')) {
    define('ION_TEST_SERVER_IPV6', "[::1]:8976");
}
if (!defined('ION_TEST_SERVER_UNIX')) {
    define('ION_TEST_SERVER_UNIX', "/tmp/test.sock");
}

function drop() {
    call_user_func_array("var_dump", func_get_args());
    $e = new Exception();
    echo "-------\nDump trace: \n" . $e->getTraceAsString() . "\n";
    exit();
}

function dump() {
    foreach (func_get_args() as $arg) {
        fwrite(STDERR, "DUMP: " . call_user_func("print_r", $arg, true) . "\n");

    }
}