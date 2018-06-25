
#ifndef ION_ION_EVENT_H
#define ION_ION_EVENT_H

extern ZEND_API zend_class_entry * ion_ce_ION_EventAbstract;

zend_object * ion_php_event_init(zend_class_entry * ce);
void ion_php_event_free(zend_object * object);

void ion_php_event_callback(evutil_socket_t fd, short flags, void * arg);
void ion_php_event_enable(ion_php_event * php_event, zend_bool promised);

#define ION_PHP_EVENT_PERSIST  (1<<0)

#define ION_PHP_EVENT_ENABLE  (1<<4)
#define ION_PHP_EVENT_ONCE  (1<<5)
#define ION_PHP_EVENT_READ  (1<<6)
#define ION_PHP_EVENT_WRITE  (1<<7)

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

#endif //ION_ION_EVENT_H
