<?php


$slist = new \ION\Data\SkipList();
$alist = new \ArrayObject();
$plist = new \SplPriorityQueue();
$plist->setExtractFlags($plist::EXTR_BOTH);
//		$this->assertSame(0, $slist->count());
$memory = memory_get_usage(1);
$top = $item = null;
$i = 0;
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$slist->add($i, "zero#$i");
}
printf("\nset slist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));

$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$alist[$i] = "zero#$i";
}
printf("\nnset alist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));

$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$plist->insert("zero#$i", -$i);
}
printf("\nnset plist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));


$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$top = $slist->first();
}
printf("\ntop slist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));

$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$alist->asort();
	$top = key($alist);
}
printf("\ntop alist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));

$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$top = $plist->top();
}
printf("\ntop plist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));

$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$item = $slist->get($i);
}
printf("\nget slist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));

$memory = memory_get_usage(1);
$time = microtime(1);
for($i=0; $i<1000; $i++) {
	$item = $alist[$i];
}
printf("\nget alist: time: %f, memory: %d KB\n", microtime(1) - $time, round((memory_get_usage(1) - $memory)/1000, 2));