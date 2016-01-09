<?php

namespace ION\HTTP\Content;


use ION\Deferred;
use ION\HTTP\Content\MultiParted\Part;
use ION\Stream;

class MultiParted {

	public function __construct(Stream $connect, string $boundary) {}

	/**
	 * @return Deferred|Part
	 */
	public function readPart() : Deferred {}

	public function writePart(Part $part) {}
}