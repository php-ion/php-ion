#ifndef ION_DEFERRED_H
#define ION_DEFERRED_H

#include <php.h>
#include <event.h>
#include "../pion.h"

BEGIN_EXTERN_C();


#define DEFERRED_RESOLVED  1
#define DEFERRED_FAILED    2
#define DEFERRED_FINISHED  3

#define DEFERRED_INTERNAL  4
#define DEFERRED_TIMED_OUT 8
#define DEFERRED_REJECTED  16

typedef struct _IONDeferred IONDeferred;

typedef void (*deferredCancelFunc)(zval *error, void *IONDeferred TSRMLS_DC);

struct _IONDeferred {
    zend_object     std;
    short            flags;
    void             (*deferredCancelFunc)(zval *error, void *IONDeferred TSRMLS_DC);
    void             *object;
    zend_class_entry *scope;
    struct event     *timeout;
    zval             *result;
    pionCb           *finish_cb;
    pionCb           *cancel_cb;
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
