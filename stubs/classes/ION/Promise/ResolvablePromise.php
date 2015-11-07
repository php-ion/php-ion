<?php

namespace ION\Promise;


use ION\Promise;

class ResolvablePromise extends Promise {

    /**
     * @param mixed $data
     */
    public function done($data) {}

    /**
     * @param \Exception $error
     */
    public function fail(\Exception $error) {}
}