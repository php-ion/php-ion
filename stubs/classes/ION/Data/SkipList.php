<?php

namespace ION\Data;

/**
 * Skip list implementation.
 * @see     http://en.wikipedia.org/wiki/Skip_list
 * @package php-ion
 */
class SkipList implements \Countable {

    const EXTR_DATA = 1;
    const EXTR_KEY  = 2;
    const EXTR_BOTH = 3;

    /**
     * Get the first pair from the skiplist.
     * @return mixed
     */
    public function first() { }

    /**
     * Get the ast pair from the skiplist.
     * @return mixed
     */
    public function last() { }

    /**
     * Pop the key/value pair off the skiplist with the first key.
     * Same return behavior as first(), but also deletes the pair.
     * @return mixed
     */
    public function lPop() { }

    /**
     * Pop the key/value pair off the skiplist with the last key.
     * Same return behavior as last(), but also deletes the pair.
     * @return mixed
     */
    public function rPop() { }

    /**
     * Set a key/value pair in the skiplist, replacing an existing
     * value if present. If OLD is non-NULL, then
     * old will be set to the previous value, or NULL if it was not present.
     *
     * @param mixed $key
     * @param mixed $value
     *
     * @return mixed
     */
    public function set($key, $value) { }

    /**
     * Add a key/value pair to the skiplist. Equal keys will be kept
     * (bag functionality).
     * If you add multiple values under the same key, they will not
     * necessarily be stored in any particular order.
     *
     * @param mixed $key
     * @param mixed $value
     *
     * @return int new count of elements in list
     */
    public function add($key, $value) { }

    /**
     * Does the skiplist contain KEY?
     *
     * @param mixed $key
     *
     * @return bool
     */
    public function exists($key) { }

    /**
     * @param mixed $key
     * @param bool $all
     *
     * @return mixed
     */
    public function get($key, $all = false) { }

    /**
     * @param string $key
     *
     * @return array
     */
    public function getAll($key) { }

    public function delete($key, $all = false) { }

    public function clear() { }

    public function getRange($min, $max = null) { }

    /**
     * @param mixed $key
     *
     * @return int
     */
    public function countKey($key) { }


    /**
     * @return int
     */
    public function count() { }

    /**
     * @return array
     */
    public function toArray() { }

    /* JsonSerializable method */
//	function jsonSerialize() {}

    /* Serializable method */
//	public function serialize() {}
//	public function unserialize($serialized) {}
}