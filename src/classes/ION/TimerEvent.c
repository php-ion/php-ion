#include "ion.h"

zend_object_handlers ion_oh_ION_TimerEvent;
zend_class_entry * ion_ce_ION_TimerEvent;


/** public static function ION\TimerEventt::__construct(double $somewhat, int $flags = 0) */
CLASS_METHOD(ION_TimerEvent, __construct) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    double          time      = 0.0;
    zend_long       flags     = 0;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_DOUBLE(time)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    php_event->type = ION_TIMER_EVENT;
    php_event->flags = (int)flags;
    ZVAL_DOUBLE(&php_event->context, time);

    ION_PHP_EVENT_CHECK(php_event->event);

    php_event->event = event_new(GION(base), -1, EV_TIMEOUT, ion_php_event_callback, php_event);
    ION_PHP_EVENT_ENSURE_ENABLE(php_event);
}

METHOD_ARGS_BEGIN(ION_TimerEvent, __construct, 1)
    ARGUMENT(time, IS_DOUBLE)
    ARGUMENT(flags, IS_LONG)
METHOD_ARGS_END();


METHODS_START(methods_ION_TimerEvent)
    METHOD(ION_TimerEvent, __construct,  ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_TimerEvent) {
    ion_register_class_ex(&ion_ce_ION_TimerEvent, ion_ce_ION_EventAbstract, "ION\\TimerEvent", ion_sequence_zend_init, methods_ION_TimerEvent);

    ion_init_object_handlers(ion_oh_ION_TimerEvent);
    ion_oh_ION_TimerEvent.free_obj = ion_php_event_free;
    ion_oh_ION_TimerEvent.offset = ion_offset(ion_php_event);

    return SUCCESS;
}
