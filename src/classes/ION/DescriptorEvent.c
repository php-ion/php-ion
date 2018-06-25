#include "ion.h"

zend_object_handlers ion_oh_ION_DescriptorEvent;
zend_class_entry * ion_ce_ION_DescriptorEvent;


/** public statizc function ION\DescriptorEvent::__construct(mixed $fd, int $flags = 0) */
CLASS_METHOD(ION_DescriptorEvent, __construct) {
    ion_php_event * php_event = ION_THIS_OBJECT(ion_php_event);
    zval          * zfd       = NULL;
    zend_long       flags     = ION_PHP_EVENT_READ | ION_PHP_EVENT_WRITE;
    int             fd = -1;
    short           event_flags = EV_READ | EV_WRITE;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(zfd)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    php_event->type = ION_DESCRIPTOR_EVENT;
    php_event->flags = (int)flags;


    if (Z_TYPE_P(zfd) == IS_RESOURCE) {
        php_stream * stream_resource;
        php_stream_from_zval_no_verify(stream_resource, zfd);
        if (stream_resource) {
            if(php_stream_cast(stream_resource, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL | PHP_STREAM_AS_SOCKETD, (void *) &fd, 0) == FAILURE) {
                zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_STREAM_RESOURCE_INVALID, 0);
                return;
            } else if (php_stream_cast(stream_resource, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, 0) == FAILURE) {
                zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_STREAM_RESOURCE_INVALID, 0);
                return;
            }
        }
    } else if (Z_TYPE_P(zfd) == IS_OBJECT) {
        // todo
        return;
    } else {
        return;
    }

    if (!(flags & ION_PHP_EVENT_ONCE)) {
        event_flags |= EV_PERSIST;
        php_event->flags |= ION_PHP_EVENT_PERSIST;
    }
    if (!(flags & (ION_PHP_EVENT_READ | ION_PHP_EVENT_WRITE))) {
        zend_throw_exception(ion_ce_InvalidArgumentException, "Event type (READ or/and WRITE) mismatch", 0);
        return;
    }
    if (flags & ION_PHP_EVENT_READ) {
        event_flags |= EV_READ;
    }
    if (flags & ION_PHP_EVENT_WRITE) {
        event_flags |= EV_WRITE;
    }
    ZVAL_COPY(&php_event->context, zfd);
    php_event->event = event_new(GION(base), fd, event_flags, ion_php_event_callback, php_event);

    ion_php_event_enable(php_event, true);
}

METHOD_ARGS_BEGIN(ION_DescriptorEvent, __construct, 1)
    ARGUMENT(fd, IS_MIXED)
    ARGUMENT(flags, IS_LONG)
METHOD_ARGS_END();


METHODS_START(methods_ION_DescriptorEvent)
    METHOD(ION_DescriptorEvent, __construct,  ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_DescriptorEvent) {
    ion_register_class_ex(&ion_ce_ION_DescriptorEvent, ion_ce_ION_EventAbstract, "ION\\DescriptorEvent", ion_sequence_zend_init, methods_ION_DescriptorEvent);

    ion_class_declare_constant_long(ion_ce_ION_DescriptorEvent, "READ",  ION_PHP_EVENT_READ);
    ion_class_declare_constant_long(ion_ce_ION_DescriptorEvent, "WRITE", ION_PHP_EVENT_WRITE);

    ion_init_object_handlers(ion_oh_ION_DescriptorEvent);
    ion_oh_ION_DescriptorEvent.free_obj = ion_php_event_free;
    ion_oh_ION_DescriptorEvent.offset = ion_offset(ion_php_event);


    return SUCCESS;
}
