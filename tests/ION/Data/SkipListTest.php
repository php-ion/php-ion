<?php

namespace ION\Data;

use ION\TestCase;

class SkipListTest extends TestCase {
	/**
	 * @memcheck
	 */
	public function testCreate() {
		$list = new SkipList();
		$this->assertNULL($list->lPop());
		$this->assertNULL($list->first());
		$this->assertNULL($list->rPop());
		$this->assertNULL($list->last());
		$this->assertSame(0, $list->count());
		$this->assertSame([], $list->toArray());
		unset($list);
	}

	public function providerSetItem() {
		return [
			[ [ ["one", 1] ] , ["one", 1]],
			[ [ ["one", 1], ["one", 2] ], ["one", 2] ],
			[ [ ["one", 1], ["one", 2], ["one", 3] ], ["one", 3] ]
		];
	}

	/**
	 * @memcheck
	 * @group testSetItem
	 * @dataProvider providerSetItem
	 * @param $items
	 * @param $result
	 */
	public function testSetItem($items, $result) {
		$list = new SkipList();
		foreach($items as $item) {
			$list->set($item[0], $item[1]);
		}
		$this->assertEquals(1, $list->count());
		$this->assertEquals($result, $list->first());
		$this->assertEquals($result, $list->last());
		$this->assertEquals($result, $list->lPop());
		$this->assertEquals(0, $list->count());
		$this->assertNULL($list->rPop());
		$this->assertEquals(0, $list->count());
		unset($list);
	}

	/**
	 * @memcheck
	 */
	public function testGet() {
		$list = new SkipList();
		$list->add("one", 1);
		$list->add("two", 2);
		$list->add("three", 3);
		$list->add("one", "more 1");

		$this->assertEquals(2, $list->get("two"));
		$this->assertEquals(3, $list->get("three"));
		$this->assertTrue(in_array($list->get("one"),  [1, "more 1"]), "get from multiple variants");
		unset($list);
	}

	/**
	 * @mem check
	 * @group testDuplicates
	 */
	public function testDuplicates() {

	}
}