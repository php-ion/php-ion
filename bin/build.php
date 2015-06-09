#!/usr/bin/env php
<?php

$options = getopt("hacimdt::g:", ["help", "clean", "info", "make", "build", "test::", "group:"]);
extract($options);

chdir(dirname(__DIR__).'/src');

if(isset($build) || isset($b)) {
	run('phpize');
	run('./configure --with-ion');
	run('make clean && make');
} elseif(isset($clean) || isset($c)) {
	run('make clean');
} elseif(isset($make) || isset($m)) {
	run('make clean && make');
} elseif(isset($help) || isset($h)) {
	echo "Usage: ".basename(__FILE__)." [--help|-h] [--clean|-m] [--make|-m] [--build|-b] [--info|-i] [--test[=TEST_PATH] [--group=GROUP_LIST]]\n";
	exit;
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
	run(PHP_BINARY." -dextension=".dirname(__DIR__)."/src/modules/ion.so vendor/bin/phpunit $group --colors=always ".select($test, $t));
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