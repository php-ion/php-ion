#ifndef ION_DEFERRED_H
#define ION_DEFERRED_H

#include <php.h>
#include <event.h>
#include "../pion.h"

BEGIN_EXTERN_C();

typedef struct _ion_deferred ion_deferred;

struct _ion_deferred {
    zend_object      std;
    short            flags;
    void             (* reject)(zval * error, zval *zdeferred TSRMLS_DC);
    void             * object;
    void             (* object_dtor)(void * object, zval *zdeferred TSRMLS_DC);
    zval             * result;
    zend_class_entry * scope;
    struct event     * ttl;
    pion_llist       * resolve;
    pion_llist       * progress;
    pionCb           * finish_cb;
    pionCb           * cancel_cb;
#ifdef ZTS
    void ***thread_ctx;
#endif
};

DEFINE_CLASS(ION_Deferred);
DEFINE_CLASS(ION_Deferred_RejectException);
DEFINE_CLASS(ION_Deferred_TimeoutException);

CLASS_INSTANCE_DTOR(ION_Deferred);
CLASS_INSTANCE_CTOR(ION_Deferred);

END_EXTERN_C();

#endif //ION_DEFERRED_H
