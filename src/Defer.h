#ifndef ION_DEFER_H
#define	ION_DEFER_H

#include <php.h>
#include "framework.h"
BEGIN_EXTERN_C();

#define DEFER_DONE      1
#define DEFER_FAIL      2
#define DEFER_FINISHED  3

#define DEFER_INTERNAL  4
#define DEFER_TIMEOUT   8
#define DEFER_CANCELED  16

#define DEFER_ALIVE     32
#define DEFER_USERSPACE 64

PHP_MINIT_FUNCTION(ion_defer);
PHP_RSHUTDOWN_FUNCTION(ion_defer);

DEFINE_CLASS(Defer);


typedef void (*cancel_func)(zval *error, void *arg TSRMLS_DC);

typedef struct _ion_defer_callback {
    phpCb *cb;
    zval *arg;
#ifdef ZTS
	void ***thread_ctx;
#endif
} IONDeferCb;

typedef struct _ion_defer {
    zend_object std;
    short       flags;
    void        (*cancel_func)(zval *error, void *arg TSRMLS_DC);
    void        *obj;
    struct event *timeout;
    //int         timeout;
    zval        *result;
    //struct _ion_defer    *parent;
    //HashTable   *childs;
    IONDeferCb  *cb;
#ifdef ZTS
    void ***thread_ctx;
#endif
} IONDefer;

// Shortcuts
#define Z_DEFER_P(zobj)     ((IONDefer *)zend_object_store_get_object(zobj TSRMLS_CC))

#define DEFERCB_FREE(callback) \
    CB_FREE(callback->cb);    \
    zval_ptr_dtor(&callback->arg); \
    efree(callback);


// C API
zval* _ion_defer_ctor(cancel_func fn, void *data);
void  ion_defer_set_cancel_cb(IONDefer *defer, cancel_func fn, void *data);
void  _ion_defer_cancel(IONDefer *defer, char *msg TSRMLS_DC);
void  _ion_defer_finish(short type, IONDefer *defer, zval *result TSRMLS_DC);

#define ion_defer_ctor(cancel_func, data)     _ion_defer_ctor(cancel_func, (void *)data TSRMLS_CC)
#define ion_defer_init()                      ion_defer_ctor(NULL, NULL)

#define ion_defer_cancel(defer, msg)          _ion_defer_cancel(defer, msg TSRMLS_CC)
#define ion_zdefer_cancel(zdefer, msg)        ion_defer_cancel(Z_DEFER_P(zdefer), msg)

#define ion_defer_finish(flag, defer, result) _ion_defer_finish(flag, defer, result TSRMLS_CC)
#define ion_defer_done(defer, result)         ion_defer_finish(DEFER_DONE, defer, result)
#define ion_zdefer_done(zdefer, result)       ion_defer_done(Z_DEFER_P(zdefer), result)
#define ion_defer_fail(defer, exception)      ion_defer_finish(DEFER_FAIL, defer, exception)
#define ion_zdefer_fail(zdefer, exception)    ion_defer_fail(Z_DEFER_P(zdefer), exception)

//#define ion_defer_dtor(zdefer)                zval_ptr_dtor(&zdefer)

END_EXTERN_C();

#endif	/* ION_DEFER_H */

