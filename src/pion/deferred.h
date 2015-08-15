#ifndef PION_DEFERRED_H
#define PION_DEFERRED_H

#include <php.h>

#define DEFERRED_RESOLVED  1
#define DEFERRED_FAILED    2
#define DEFERRED_FINISHED  3

#define DEFERRED_INTERNAL  4
#define DEFERRED_TIMED_OUT 8
#define DEFERRED_REJECTED  16
#define DEFERRED_AUTO_UNREF 32

typedef void (*reject_callback)(zval *error, void *IONDeferred TSRMLS_DC);
typedef void (*object_dtor)(void *object TSRMLS_DC);

zval * ionDeferredNew(zval *z_reject_cb TSRMLS_DC);
zval * ionDeferredNewInternal(reject_callback reject_cb, short auto_unref TSRMLS_DC);
int    ionDeferredCast(zval * zvariable, void * object, object_dtor dtor, reject_callback reject_cb TSRMLS_DC);
void   ionDeferredStore(zval * zDeferred, void * object, object_dtor dtor TSRMLS_DC);
void * ionDeferredStoreGet(zval *zDeferred TSRMLS_DC);
void   ionDeferredFinish(zval * zDeferred, zval * zResult, short type TSRMLS_DC);
void   ionDeferredReject(zval * zDeferred, const char * message TSRMLS_DC);
void   ionDeferredFree(zval * zDeferred TSRMLS_DC);

#define deferredNew(zcancel_cb)                        ionDeferredNew(cancel_cb TSRMLS_CC)
#define deferredNewInternal(cancel_cb, auto_unref)     ionDeferredNewInternal(cancel_cb, auto_unref TSRMLS_CC)
//#define deferredCast(zvariable, object, dtor, cancel_cb) ionDeferredCast(zvariable, object, dtor, cancel_cb TSRMLS_CC)
#define deferredStore(zdeferred, object, object_dtor)  ionDeferredStore(zdeferred, (void *) object, object_dtor TSRMLS_CC)
#define deferredStoreGet(zdeferred)                    ionDeferredStoreGet(zdeferred TSRMLS_CC)
#define deferredResolve(zdeferred, zResult)            ionDeferredFinish(zdeferred, zResult, DEFERRED_RESOLVED TSRMLS_CC)
#define deferredError(zdeferred, zException)           ionDeferredFinish(zdeferred, zException, DEFERRED_FAILED TSRMLS_CC)
#define deferredReject(zdeferred, message)             ionDeferredReject(zdeferred, message TSRMLS_CC)
#define deferredFree(zdeferred)                        ionDeferredFree(zdeferred TSRMLS_CC)

#endif //PION_DEFERRED_H
