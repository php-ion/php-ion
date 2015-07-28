#!/usr/bin/env php
<?php

$options = getopt("hacimdtp::g:", ["help", "phpize", "clean", "info", "make", "build", "test::", "group:"]);
extract($options);

if(isset($help) || isset($h)) {
	echo "Usage: ".basename(__FILE__)." [--help|-h] [--clean|-c] [--make|-m] [--phpize|-p] [--build|-b] [--info|-i] [--test[=TEST_PATH] [--group=GROUP_LIST]]\n";
	exit;
}

chdir(dirname(__DIR__).'/src');

if(isset($build) || isset($b)) {
	$phpize = true;
	$clean  = true;
	$make   = true;
}

if(isset($phpize) || isset($p)) {
	run('phpize');
	run('./configure --with-ion');
}

if(isset($clean) || isset($c)) {
	run('make clean');
}

if(isset($make) || isset($m)) {
	run('make');
}

if(isset($info) || isset($i)) {
	run(PHP_BINARY . ' -dextension=modules/ion.so --ri ION');
}

if(isset($test) || isset($t)) {
	chdir(dirname(__DIR__));
	if(isset($group) || isset($g)) {
		$group = "--group=".select($group, $g);
	} else {
		$group = "";
	}
	if(getenv('CLION_BUILD')) {
		$colorize = '--colors=never';
	} else {
		$colorize = '--colors=always';
	}
	run(PHP_BINARY." -dextension=".dirname(__DIR__)."/src/modules/ion.so vendor/bin/phpunit $group $colorize ".select($test, $t));
}


function select(&$long, &$short) {
	if($long === null) {
		return $short;
	} else {
		return $long;
	}
}

function run($cmd) {
	passthru($cmd.' 2>&1', $code);
	return $code == 0;
}