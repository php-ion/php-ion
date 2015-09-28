#ifndef ION_PROMISE_H
#define ION_PROMISE_H

#include <php.h>
#include <event.h>
#include "../pion.h"

BEGIN_EXTERN_C();

typedef struct _ion_promise_chain {
    ushort           flags;
} ion_promise_chain;

typedef struct _ion_promise {
    zend_object        std;
    ushort             flags;
    pionCb           * done;
    pionCb           * fail;
    pionCb           * progress;
    zval             * await;
    zval             * result;
    zval             * yield;
    struct event     * ttl;
    zval             * parent;
    zval            ** childs;
    uint childs_count;
    pion_llist       * next;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_promise;

DEFINE_CLASS(ION_Promise);

CLASS_INSTANCE_DTOR(ION_Promise);
CLASS_INSTANCE_CTOR(ION_Promise);

END_EXTERN_C();

#endif //ION_PROMISE_H
