#ifndef ION_PROMISE_H
#define ION_PROMISE_H

#include <php.h>
#include <event.h>
#include "../pion.h"

BEGIN_EXTERN_C();

typedef struct _ion_promise {
    zend_object        std;
#ifdef ION_DEBUG
    long               uid; // to distinguish promise-objects
#endif
    ushort             flags;
    pionCb           * done;
    pionCb           * fail;
    pionCb           * progress;
    zval             * await;
    zval             * result;
    zval             * generator;
    zval             * generator_result;
    zval            ** generators_stack;
    ushort             generators_count;
    struct event     * ttl;
    zval            ** handlers;
    ushort             handler_count;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_promise;

zend_class_entry * ion_get_class(ION_Promise);

END_EXTERN_C();

#endif //ION_PROMISE_H
