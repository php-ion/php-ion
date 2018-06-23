#include "ion.h"

zend_object_handlers ion_oh_ION_SignalEvent;
zend_class_entry * ion_ce_ION_SignalEvent;


/** public static function ION\SignalEvent::__construct(int $signal, int $flags = 0) */
CLASS_METHOD(ION_SignalEvent, __construct) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    zend_long    signal       = 0;
    zend_long    flags        = 0;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(signal)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    php_event->type = ION_SIGNAL_EVENT;
    php_event->flags = (int)flags;
    ZVAL_LONG(&php_event->context, signal);

    ION_PHP_EVENT_CHECK(php_event->event);

    php_event->event = event_new(GION(base), (int)signal, EV_SIGNAL, ion_php_event_callback, php_event);
    ION_PHP_EVENT_ENSURE_ENABLE(php_event);
}

METHOD_ARGS_BEGIN(ION_SignalEvent, __construct, 1)
    ARGUMENT(time, IS_LONG)
    ARGUMENT(flags, IS_LONG)
METHOD_ARGS_END();


METHODS_START(methods_ION_SignalEvent)
    METHOD(ION_SignalEvent, __construct,  ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_SignalEvent) {
    ion_register_class_ex(&ion_ce_ION_SignalEvent, ion_ce_ION_EventAbstract, "ION\\SignalEvent", ion_sequence_zend_init, methods_ION_SignalEvent);

    ion_init_object_handlers(ion_oh_ION_SignalEvent);
    ion_oh_ION_SignalEvent.free_obj = ion_php_event_free;
    ion_oh_ION_SignalEvent.offset = ion_offset(ion_php_event);

    return SUCCESS;
}
