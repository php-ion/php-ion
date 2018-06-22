
#ifndef ION_ION_EVENT_H
#define ION_ION_EVENT_H

extern ZEND_API zend_class_entry * ion_ce_ION_EventAbstract;

zend_object * ion_php_event_init(zend_class_entry * ce);
void ion_php_event_free(zend_object * object);

void ion_php_event_callback(evutil_socket_t fd, short flags, void * arg);
void ion_php_event_enable(ion_php_event * php_event, zend_bool internal);

#define ION_PHP_EVENT_ENABLE  (1<<0)
#define ION_PHP_EVENT_TIMER_ONCE  (1<<1)

typedef enum _ion_php_event_type {
    ION_DESCRIPTOR_EVENT = 1,
    ION_TIMER_EVENT = 2,
    ION_SIGNAL_EVENT = 3
} ion_php_event_type;

typedef struct _ion_php_event {
    ion_php_event_type   type;
    ion_event          * event;
    ion_promisor       * promise;
    int                  flags;
    zval                 context;

    zend_object   php_object;
};

#define ION_PHP_EVENT_CHECK(event)                                                      \
    if (!(event)) {                                                                     \
        zend_throw_exception(ion_ce_ION_RuntimeException, ERR_ION_EVENT_NOT_READY, 0);  \
        return;                                                                         \
    }

#define ION_PHP_EVENT_ENSURE_ENABLE(php_event)      \
    if (php_event->flags & ION_PHP_EVENT_ENABLE) {  \
        ion_php_event_enable(php_event, true);      \
    }


#endif //ION_ION_EVENT_H
