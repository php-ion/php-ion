#include "callback.h"
#include "deferred_queue.h"
#include "engine.h"

ion_global_queue * ion_deferred_queue_init() {
    ion_global_queue * queue = ecalloc(1, sizeof(ion_global_queue));
    queue->event = event_new(GION(base), -1, EV_TIMEOUT, ion_deferred_dequeue_event_wrapper, NULL);
    return queue;
}

void ion_deferred_queue_free(ion_global_queue * queue) {
    ion_deferred_clean();
    event_del(queue->event);
    event_free(queue->event);
    efree(queue);
}

void ion_deferred_queue_push(pion_cb * cb, zend_object * object) {
    ion_global_queue * queue = GION(queue);
    if(!queue->size) {
        queue->size = 16;
        queue->queue = ecalloc(queue->size, sizeof(ion_global_queue_item));
    }
    if(queue->size == queue->length) {
        queue->size *= 2;
        queue->queue = erealloc(queue->queue, queue->size * sizeof(ion_global_queue_item));
    }
    zend_object_addref(object);
    size_t pos = queue->length++;
    queue->queue[pos].cb = cb;
    queue->queue[pos].object = object;

    if(!GION(queue)->delayed) {
        ion_time tv = {0, 0};

        event_add(GION(queue)->event, &tv);
        GION(queue)->delayed = true;
    }
}

void ion_deferred_dequeue() {
    if(GION(queue)->length) {
        ion_global_queue * queue = GION(queue);

        for(size_t i = 0; i < queue->length; i++) {
            pion_cb * cb = queue->queue[i].cb;
            zend_object * object = queue->queue[i].object;
            if(!object) {
                continue;
            }
            zval result = pion_cb_obj_call_without_args(cb, object);
            zval_ptr_dtor(&result);
            pion_cb_release(cb);
            zend_object_release(object);
            if(EG(exception)) {
                queue->queue[i].object = NULL; // todo: realloc queue
            }
        }
        efree(queue->queue);
        queue->size = queue->length = 0;
    }
}

void ion_deferred_dequeue_event_wrapper(evutil_socket_t fd, short flags, void * arg) {
    ION_CB_BEGIN();
    ion_deferred_dequeue();
    ION_CB_END();
}

void ion_deferred_clean() {
    if(GION(queue)->length) {
        ion_global_queue * queue = GION(queue);
        for(size_t i = 0; i < queue->length; i++) {
            if(!queue->queue[i].object) {
                continue;
            }
            pion_cb_release(queue->queue[i].cb);
            zend_object_release(queue->queue[i].object);
        }
        efree(queue->queue);
        queue->size = queue->length = 0;
    }
}

