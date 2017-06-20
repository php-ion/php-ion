#ifndef ION_DEFERRED_QUEUE_H
#define ION_DEFERRED_QUEUE_H

typedef struct _ion_global_queue_item {
    pion_cb     * cb;
    zend_object * object;
} ion_global_queue_item;


struct _ion_global_queue {
    size_t      size;
    size_t      length;
    ion_global_queue_item * queue;
    ion_event * event;
    zend_bool   delayed;
};

ion_global_queue * ion_deferred_queue_init();
void ion_deferred_queue_free(ion_global_queue * queue);
void ion_deferred_queue_push(pion_cb * cb, zend_object * object);
void ion_deferred_dequeue();
void ion_deferred_dequeue_event_wrapper(evutil_socket_t fd, short flags, void * arg);
void ion_deferred_clean();

#endif //ION_DEFERRED_QUEUE_H
