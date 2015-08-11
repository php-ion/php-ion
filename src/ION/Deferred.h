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
typedef void (*object_dtor)(void *object TSRMLS_DC);


struct _IONDeferred {
    zend_object     std;
    short            flags;
    void             (*deferredCancelFunc)(zval *error, void *IONDeferred TSRMLS_DC);
    void             *object;
    void             (*object_dtor)(void *object TSRMLS_DC);
    zend_class_entry *scope;
    struct event     *timeout;
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

zval * ionDeferredNew(zval *zCancelCb TSRMLS_DC);
zval * ionDeferredNewInternal(deferredCancelFunc, void *, object_dtor TSRMLS_DC);
//void ionDeferredStorePut(zval *zDeferred, void *object, object_dtor * TSRMLS_DC);
void * ionDeferredStoreGet(zval *zDeferred TSRMLS_DC);
void ionDeferredFinish(zval *zDeferred, zval *zResult, short type TSRMLS_DC);
void ionDeferredReject(zval *zDeferred, const char *message TSRMLS_DC);

#define deferredNew(cancel_cb)                  ionDeferredNew(cancel_cb TSRMLS_DC)
#define deferredNewInternal(cancel_cb, object, object_dtor)  ionDeferredNewInternal(cancel_cb, (void *) object, object_dtor TSRMLS_DC)
#define deferredStoreGet(zDeferred)             ionDeferredStoreGet(zDeferred TSRMLS_DC)
#define deferredResolve(zDeferred, zResult)     ionDeferredFinish(zDeferred, zResult, DEFERRED_RESOLVED TSRMLS_DC)
#define deferredError(zDeferred, zException)    ionDeferredFinish(zDeferred, zException, DEFERRED_FAILED TSRMLS_DC)
#define deferredReject(zDeferred, message)      ionDeferredReject(zDeferred, message TSRMLS_DC)


END_EXTERN_C();

#endif //ION_DEFERRED_H
