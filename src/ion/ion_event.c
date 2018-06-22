#include "ion.h"

void ion_php_event_callback(evutil_socket_t fd, short flags, void * arg) {
    ION_CB_BEGIN();
    ion_php_event * php_event = (ion_php_event *) arg;

    pion_cb * cb = pion_cb_create_from_object(ION_OBJECT_ZOBJ(php_event), "_occurred");
    pion_cb_void_with_1_arg(cb, &php_event->context);
    pion_cb_release(cb);

    ION_CB_END();
}

void ion_php_event_enable(ion_php_event * php_event, zend_bool internal) {
    struct timeval * ptv = NULL;
    struct timeval   tv;

    if (php_event->type == ION_TIMER_EVENT) {
        tv.tv_usec = ((int)(Z_DVAL(php_event->context)*1000000) % 1000000);
        tv.tv_sec  = (int)Z_DVAL(php_event->context);
        ptv = &tv;
    }
    if(event_add(php_event->event, ptv) == FAILURE) {
        if (internal) {
            if (php_event->promise) {
                ion_promisor_throw(php_event->promise, ion_ce_ION_RuntimeException, ERR_ION_EVENT_ADD, 0);
            }
        } else {
            zend_throw_exception(ion_ce_ION_RuntimeException, ERR_ION_EVENT_ADD, 0);
        }
        return;
    }
    php_event->flags |= ION_PHP_EVENT_ENABLE;
}