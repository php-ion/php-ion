<?php

namespace ION\Data;

use ION\TestCase;

class LinkedListTest extends TestCase {
	/**
	 * @memcheck
	 */
	public function testCreate() {
		$list = new LinkedList();
		$this->assertNULL($list->lPop());
		$this->assertNULL($list->rPop());
		$this->assertSame(0, $list->count());
		unset($list);
	}

	/**
	 * @memcheck
	 */
	public function testOneItem() {
		$list = new LinkedList();
		$list->rPush("one");
		$this->assertSame(1, $list->count());
		$this->assertSame("one", $list->lPop());
		$this->assertNULL($list->rPop());
		$this->assertSame(0, $list->count());
		unset($list);
	}

	/**
	 * @memcheck
	 */
	public function testSomeItems() {
		$items = ["a", "b", "c"];
		$list = new LinkedList();
		$this->assertSame(0, $list->count());
		$list->lPush("zero");
		foreach($items as $item) {
			$list->rPush($item);
		}
		$this->assertSame(4, $list->count());
		$this->assertSame("zero", $list->lPop());
		$this->assertSame(3, $list->count());
		$this->assertSame("a", $list->lPop());
		$this->assertSame(2, $list->count());
		$this->assertSame("c", $list->rPop());
		$this->assertSame(1, $list->count());
		$this->assertSame("b", $list->rPop());
		$this->assertSame(0, $list->count());
	}
	/**
	 * @memcheck
	 */
	public function testDtor() {
		$list = new LinkedList();
		$list->rPush("one");
		$list->lPush("two");
	}

	/**
	 * @memcheck
	 */
	public function testIterator() {
		$result = [];
		$list = new LinkedList();
		$list->rPush("one");
		$list->lPush("two");
		foreach ($list as $key => $item) {
			$result[$key] = $item;
		}
		$this->assertSame(2, $list->count());
		$this->assertSame(["two", "one"], $result);
	}

	/**
	 * @memcheck
	 */
	public function testIncreaseIterator() {
		$result = [];
		$data = ["two", "three", "four"];
		$list = new LinkedList();
		$list->rPush("one");
		foreach ($list as $key => $item) {
			$result[$key] = $item;
			if(count($data)) {
				$list->rPush(array_shift($data));
			}
		}
		$this->assertSame(4, $list->count());
		$this->assertSame(["one", "two", "three", "four"], $result);
	}

	/**
	 * @mem check
	 * @group testStack
	 */
	public function testStack() {
		$list = new LinkedList();
		$data = ["two", "three", "four"];
		foreach($data as $item) {
			$list->lPush($item);
		}
		$this->assertSame(3, $list->count());
		$data = array_reverse($data);
		foreach($data as $item) {
			$this->assertSame($item, $i =$list->lPop());
		}
		$this->assertNull($list->lPop());
		$this->assertSame(0, $list->count());
	}
}