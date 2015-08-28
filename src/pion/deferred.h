#ifndef PION_DEFERRED_H
#define PION_DEFERRED_H

#include <php.h>

#define ION_DEFERRED_RESOLVED  1
#define ION_DEFERRED_FAILED    2
#define ION_DEFERRED_FINISHED  3
#define ION_DEFERRED_REJECTED  4

#define ION_DEFERRED_INTERNAL  8
#define ION_DEFERRED_TIMED_OUT 16
#define ION_DEFERRED_DALAYED   32

typedef void (*deferred_reject_callback)(zval *error, zval * zdeferred TSRMLS_DC);
typedef void (*deferred_object_dtor)(void *object, zval * zdeferred TSRMLS_DC);

zval * _ion_deferred_new(zval *z_reject_cb TSRMLS_DC);
zval * _ion_deferred_new_ex(deferred_reject_callback reject_cb TSRMLS_DC);
int    _ion_deferred_zval(zval *zvariable, void *object, deferred_object_dtor dtor, deferred_reject_callback reject_cb TSRMLS_DC);
void   _ion_deferred_store(zval *zDeferred, void *object, deferred_object_dtor dtor TSRMLS_DC);
void * _ion_deferred_store_get(zval *zDeferred TSRMLS_DC);
void   _ion_deferred_resolve(zval *zDeferred, zval *zResult, short type TSRMLS_DC);
void   _ion_deferred_reject(zval *zDeferred, const char *message TSRMLS_DC);
void   _ion_deferred_free(zval *zDeferred TSRMLS_DC);
int    _ion_deferred_dequeue(TSRMLS_DC);

#define ion_deferred_new(zcancel_cb)                        _ion_deferred_new(cancel_cb TSRMLS_CC)
#define ion_deferred_new_ex(cancel_cb)                      _ion_deferred_new_ex(cancel_cb TSRMLS_CC)
#define ion_deferred_new_void()                             _ion_deferred_new_ex(NULL TSRMLS_CC)
#define ion_deferred_zval(zvar, object, dtor, cancel_cb)    _ion_deferred_zval(zvar, object, dtor, cancel_cb TSRMLS_CC)
#define ion_deferred_store(zdeferred, object, object_dtor)  _ion_deferred_store(zdeferred, (void *) object, object_dtor TSRMLS_CC)
#define ion_deferred_store_get(zdeferred)                   _ion_deferred_store_get(zdeferred TSRMLS_CC)
#define ion_deferred_done(zdeferred, zResult)               _ion_deferred_resolve(zdeferred, zResult, ION_DEFERRED_RESOLVED TSRMLS_CC)
#define ion_deferred_fail(zdeferred, zException)            _ion_deferred_resolve(zdeferred, zException, ION_DEFERRED_FAILED TSRMLS_CC)
#define ion_deferred_reject(zdeferred, message)             _ion_deferred_reject(zdeferred, message TSRMLS_CC)
#define ion_deferred_free(zdeferred)                        _ion_deferred_free(zdeferred TSRMLS_CC)

#endif //PION_DEFERRED_H
