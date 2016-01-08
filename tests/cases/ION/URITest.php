<?php

namespace ION;


use ION\Test\TestCase;

class URITest extends TestCase {

	public function providerGets() {
		$complete_uri = "http://root:123@example.com:8080/fake/index.html?param=val#anchor";
		$relative_uri = "/fake/index.html?param=val#anchor";
		$host_uri = "http://root:123@example.com:8080";

		return array(
			[$complete_uri, 'getScheme',       'http'],
			[$complete_uri, 'getAuthority',    'root:123@example.com:8080'],
			[$complete_uri, 'getUserInfo',     'root:123'],
			[$complete_uri, 'getUserName',     'root'],
			[$complete_uri, 'getUserPassword', '123'],
			[$complete_uri, 'getHost',         'example.com'],
			[$complete_uri, 'getPort',         8080],
			[$complete_uri, 'getPath',         '/fake/index.html'],
			[$complete_uri, 'getQuery',        'param=val'],
			[$complete_uri, 'getFragment',     'anchor'],

			[$complete_uri, 'hasScheme',       true],
			[$complete_uri, 'hasUserInfo',     true],
			[$complete_uri, 'hasUserPassword', true],
			[$complete_uri, 'hasHost',         true],
			[$complete_uri, 'hasPort',         true],
			[$complete_uri, 'hasPath',         true],
			[$complete_uri, 'hasQuery',        true],
			[$complete_uri, 'hasFragment',     true],

			[$relative_uri, 'getScheme',       ''],
			[$relative_uri, 'getAuthority',    ''],
			[$relative_uri, 'getUserInfo',     ''],
			[$relative_uri, 'getUserName',     ''],
			[$relative_uri, 'getUserPassword', ''],
			[$relative_uri, 'getHost',         ''],
			[$relative_uri, 'getPort',         null],

			[$relative_uri, 'hasScheme',       false],
			[$relative_uri, 'hasUserInfo',     false],
			[$relative_uri, 'hasUserPassword', false],
			[$relative_uri, 'hasHost',         false],
			[$relative_uri, 'hasPort',         false],

			[$relative_uri, 'getPath',         '/fake/index.html'],
			[$relative_uri, 'getQuery',        'param=val'],
			[$relative_uri, 'getFragment',     'anchor'],

			[$host_uri, 'getPath',         ''],
			[$host_uri, 'getQuery',        ''],
			[$host_uri, 'getFragment',     ''],

			[$host_uri, 'hasPath',         false],
			[$host_uri, 'hasQuery',        false],
			[$host_uri, 'hasFragment',     false],

			[$host_uri, 'getScheme',       'http'],
			[$host_uri, 'getAuthority',    'root:123@example.com:8080'],
			[$host_uri, 'getUserInfo',     'root:123'],
			[$host_uri, 'getUserName',     'root'],
			[$host_uri, 'getUserPassword', '123'],
			[$host_uri, 'getHost',         'example.com'],
			[$host_uri, 'getPort',         8080],
		);
	}

	/**
	 * @dataProvider providerGets
	 * @param $uri
	 * @param $method
	 * @param $result
	 * @memcheck
	 */
	public function testGets($uri, $method, $result) {
		$uri = URI::parse($uri);
		$this->assertSame($result, $uri->$method());
	}

	/**
	 * @memcheck
	 */
	public function testToString() {
		$complete_uri = "http://root:123@example.com:8080/fake/index.html?param=val#anchor";
		$uri = URI::parse($complete_uri);
		$this->assertEquals($complete_uri, strval($uri));
	}

	/**
	 * @memcheck
	 */
	public function testClone() {
		$complete_uri = "http://root:123@example.com:8080/fake/index.html?param=val#anchor";
		$uri = URI::parse($complete_uri);
		$uri->a = 5;
		$uri2 = clone $uri;
		$this->assertNotSame($uri, $uri2);
		$this->assertEquals(5, $uri2->a);
		$uri2->a= 7;
		$this->assertEquals(5, $uri->a);
		$this->assertEquals(7, $uri2->a);
	}

	public function providerWith() {
		$complete_uri = "http://root:123@example.com:8080/fake/index.html?param=val#anchor";
		$relative_uri = "/fake/index.html?param=val#anchor";
		$host_uri = "http://root:123@example.com:8080";

		return array(
			[$complete_uri, 'withScheme', 'xxx', 'getScheme'],
			[$complete_uri, 'withUserInfo', 'xxx', 'getUserName'],
			[$complete_uri, 'withUserInfo', ['xxx', 'yyy'], 'getUserPassword'],
			[$complete_uri, 'withHost', 'xxx', 'getHost'],
			[$complete_uri, 'withPort', 9080, 'getPort'],
			[$complete_uri, 'withPath', 'xxx', 'getPath'],
			[$complete_uri, 'withQuery', 'xxx', 'getQuery'],
			[$complete_uri, 'withFragment', 'xxx', 'getFragment'],

			[$relative_uri, 'withScheme', 'xxx', 'getScheme'],
			[$relative_uri, 'withUserInfo', 'xxx', 'getUserName'],
			[$relative_uri, 'withUserInfo', ['xxx', 'yyy'], 'getUserPassword'],
			[$relative_uri, 'withHost', 'xxx', 'getHost'],
			[$relative_uri, 'withPort', 9080, 'getPort'],

			[$host_uri, 'withPath', 'xxx', 'getPath'],
			[$host_uri, 'withQuery', 'xxx', 'getQuery'],
			[$host_uri, 'withFragment', 'xxx', 'getFragment'],
		);
	}

	/**
	 * @dataProvider providerWith
	 * @memcheck
	 */
	public function testWith($uri, $method, $args, $get_method) {
		$uri = URI::parse($uri);
		$prev = $uri->$get_method();
		if(is_array($args)) {
			$uri2 = $uri->$method(...$args);
		} else {
			$uri2 = $uri->$method($args);
		}
		/* @var URI $uri2 */
		$this->assertInstanceOf('ION\\URI', $uri2);
		$this->assertNotSame($uri, $uri2);
		$this->assertSame($prev, $uri->$get_method());
		$this->assertNotSame($prev, $uri2->$get_method());
		$this->assertNotSame($uri->__toString(), $uri2->__toString());
	}

}