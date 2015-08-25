<?php
define('ION_RESOURCES', __DIR__ . "/resources");

require_once __DIR__ . "/TestCase.php";

ini_set('date.timezone', 'Europe/Moscow');

if(!defined('ION_TEST_SERVER_HOST')) {
	define('ION_TEST_SERVER_HOST', "127.0.0.1:8976");
}

function drop()
{
	call_user_func_array("var_dump", func_get_args());
	$e = new Exception();
	echo "-------\nDump trace: \n" . $e->getTraceAsString() . "\n";
	exit();
}

function dump()
{
	foreach (func_get_args() as $arg) {
		fwrite(STDERR, "DUMP: " . call_user_func("print_r", $arg, true) . "\n");

	}
}