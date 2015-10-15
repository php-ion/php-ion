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
    zval            ** handlers;
    uint               handlers_count;
    pion_cb * cancel_cb;
#ifdef ZTS
    void ***thread_ctx;
#endif
};

zend_class_entry * ion_get_class(ION_Deferred);
zend_class_entry * ion_get_class(ION_Deferred_RejectException);
zend_class_entry * ion_get_class(ION_Deferred_TimeoutException);

END_EXTERN_C();

#endif //ION_DEFERRED_H
