#ifndef ION_DEFERRED_H
#define ION_DEFERRED_H

#include <php.h>
#include <event.h>
#include "../pion.h"

BEGIN_EXTERN_C();


typedef void (*deferred_cancel_func)(zval *error TSRMLS_DC);

typedef struct _IONDeferred {
    zend_object std;
    short       flags;
    void        (*deferred_cancel_func)(zval *error TSRMLS_DC);
    void        *obj;
    struct event *timeout;
    zval        *result;
    pionCb      *cb;
#ifdef ZTS
    void ***thread_ctx;
#endif
} IONDeferred;

CLASS_INSTANCE_DTOR(ION_Deferred);
CLASS_INSTANCE_CTOR(ION_Deferred);


END_EXTERN_C();

#endif //ION_DEFERRED_H
