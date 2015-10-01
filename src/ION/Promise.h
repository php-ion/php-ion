#ifndef ION_PROMISE_H
#define ION_PROMISE_H

#include <php.h>
#include <event.h>
#include "../pion.h"

BEGIN_EXTERN_C();

typedef struct _ion_promise {
    zend_object        std;
    ushort             flags;
    pionCb           * done;
    pionCb           * fail;
    pionCb           * progress;
    zval             * await;
    zval             * result;
    zval             * generator;
    struct event     * ttl;
    zval             * parent;
    zval            ** childs;
    zval            ** pending;
    ushort             childs_count;
    ushort             pending_count;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_promise;

zend_class_entry * ion_get_class(ION_Promise);

//DEFINE_CLASS(ION_Promise);

CLASS_INSTANCE_DTOR(ION_Promise);
CLASS_INSTANCE_CTOR(ION_Promise);

END_EXTERN_C();

#endif //ION_PROMISE_H
