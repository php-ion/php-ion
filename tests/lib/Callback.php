<?php
namespace ION\Test;

class Callback {
    public $cb;
    public $name;
    public function __construct(callable $cb, $name = "cb") {
//        fwrite(STDERR, "{$this->name} construct");
        $this->cb = $cb;
        $this->name = $name;
    }

    /**
     * @return mixed
     */
    public function __invoke() {
        fwrite(STDERR, "{$this->name} invoke\n");
        if($this->cb) {
            return call_user_func_array($this->cb, func_get_args());
        } else {
            return null;
        }
    }

    public function __destruct() {
        fwrite(STDERR, "{$this->name} destruct\n");
    }
}