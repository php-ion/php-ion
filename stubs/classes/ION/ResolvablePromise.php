<?php

namespace ION;

class ResolvablePromise extends Promise {

    /**
     * @param mixed $data
     */
    public function done($data) {}

    /**
     * @param \Throwable $error
     */
    public function fail(\Throwable $error) {}
}