<?php
/**
 *
 */

namespace ION;


abstract class EventAbstract implements EventInterface
{

    public function enable() { }

    public function disable() { }

    public function then(): Sequence { }

    public function setPriority(int $prio) { }

    public function getPriority(): int { }

    public function getTarget() { }
}