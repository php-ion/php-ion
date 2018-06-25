#include "ion.h"

zend_object_handlers ion_oh_ION_EventAbstract;
zend_class_entry * ion_ce_ION_EventAbstract;



zend_object * ion_php_event_init(zend_class_entry * ce) {
    ion_php_event * pevent = ion_alloc_object(ce, ion_php_event);
    return ion_init_object(ION_OBJECT_ZOBJ(pevent), ce, &ion_oh_ION_EventAbstract);
}

void ion_php_event_free(zend_object * object) {
    ion_php_event * pevent = ION_ZOBJ_OBJECT(object, ion_php_event);
    zend_object_std_dtor(object);
    zval_ptr_dtor(&pevent->context);
    if(pevent->event) {
        event_free(pevent->event);
    }
    if(pevent->promise) {
        zend_object_release(ION_OBJECT_ZOBJ(pevent->promise));
    }

}

/** public static function ION\EventAbstract::enable() */
CLASS_METHOD(ION_EventAbstract, enable) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    ION_PHP_EVENT_CHECK(php_event->event);
    ion_php_event_enable(php_event, false);

}

METHOD_WITHOUT_ARGS(ION_EventAbstract, enable);

/** public static function ION\EventAbstract::disable() */
CLASS_METHOD(ION_EventAbstract, disable) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    if (!(php_event->flags & ION_PHP_EVENT_ENABLE)) {
        return;
    }
    ION_PHP_EVENT_CHECK(php_event->event);
    event_del(php_event->event);
}

METHOD_WITHOUT_ARGS(ION_EventAbstract, disable);

/** public static function ION\EventAbstract::then() : ION\Sequence */
CLASS_METHOD(ION_EventAbstract, then) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    if (!php_event->promise) {
        php_event->promise = ion_promisor_sequence_new(NULL);
    }

    RETURN_ION_OBJ(php_event->promise);
}

METHOD_WITHOUT_ARGS(ION_EventAbstract, then);

/** public static function ION\EventAbstract::setPriority(int $prio) */
CLASS_METHOD(ION_EventAbstract, setPriority) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    zend_long       prio = 0;
    ION_PHP_EVENT_CHECK(php_event->event);
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(prio)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
}

METHOD_ARGS_BEGIN(ION_EventAbstract, setPriority, 1)
    ARGUMENT(prio, IS_LONG)
METHOD_ARGS_END();

/** public static function ION\EventAbstract::_occurred(mixed $obj) */
CLASS_METHOD(ION_EventAbstract, _occurred) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    zval          * context   = NULL;

    if (!php_event->event) {
        if (php_event->promise) {
            ion_promisor_throw(php_event->promise, ion_ce_ION_RuntimeException, ERR_ION_EVENT_NOT_READY, 0);
        } else {
            // @todo trigger notice
        }
        return;
    }
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(context)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_promisor_done(php_event->promise, context);
}

METHOD_ARGS_BEGIN(ION_EventAbstract, _occurred, 1)
    ARGUMENT(obj, IS_MIXED)
METHOD_ARGS_END();

METHODS_START(methods_ION_EventAbstract)
    METHOD(ION_EventAbstract, enable,       ZEND_ACC_PUBLIC)
    METHOD(ION_EventAbstract, disable,      ZEND_ACC_PUBLIC)
    METHOD(ION_EventAbstract, then,         ZEND_ACC_PUBLIC)
    METHOD(ION_EventAbstract, setPriority,  ZEND_ACC_PUBLIC)
    METHOD(ION_EventAbstract, _occurred,    ZEND_ACC_PUBLIC | ZEND_ACC_PROTECTED)
METHODS_END;

PHP_MINIT_FUNCTION(ION_EventAbstract) {
    ion_register_class(ion_ce_ION_EventAbstract, "ION\\EventAbstract", ion_php_event_init, methods_ION_EventAbstract);
    ion_ce_ION_EventAbstract->ce_flags |= ZEND_ACC_ABSTRACT;

    ion_class_declare_constant_long(ion_ce_ION_EventAbstract, "ENABLE",   ION_PHP_EVENT_ENABLE);
    ion_class_declare_constant_long(ion_ce_ION_EventAbstract, "ONCE",     ION_PHP_EVENT_ONCE);

    ion_init_object_handlers(ion_oh_ION_EventAbstract);
    ion_oh_ION_EventAbstract.free_obj = ion_php_event_free;
//    ion_oh_ION_URI.clone_obj = ion_uri_clone_handler;
    ion_oh_ION_EventAbstract.offset = ion_offset(ion_php_event);

    return SUCCESS;
}



