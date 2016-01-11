<?php

namespace ION\Data;

/**
 * Implementation of doubly linked list
 * @see     http://en.wikipedia.org/wiki/Doubly_linked_list
 * @package php-ion
 */
class LinkedList implements \Iterator, \Countable {

    /**
     * Append one or multiple values to a list
     *
     * @param mixed $item
     *
     * @return int the length of the list after the push operation.
     */
    public function rPush($item) { }

    /**
     * Prepend one or multiple values to a list
     *
     * @param mixed $item
     *
     * @return int the length of the list after the push operation.
     */
    public function lPush($item) { }

    /**
     * Remove and get the last element in a list
     * @return mixed
     */
    public function rPop() { }

    /**
     * Removes and returns the first element of the list stored at key.
     * @return mixed
     */
    public function lPop() { }

    public function __clone() { }

    /* Countable method */
    /**
     * @return int
     */
    public function count() { }

    /* Iterator methods */
    public function current() { }

    public function key() { }

    public function next() { }

    public function rewind() { }

    /**
     * @return bool
     */
    public function valid() { }

    /* JsonSerializable method */
//	public function jsonSerialize() {}

    /* Serializable method */
//	public function serialize() {}
//	public function unserialize($serialized) {}
}