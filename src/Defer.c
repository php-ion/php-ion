#include "Defer.h"
#include "pion.h"

DEFINE_CLASS(Defer);
DEFINE_CLASS(CancelException);
DEFINE_CLASS(TimeoutException);


/**
 * invoke onResult callback
 * @param cb callback
 * @param result if null - defer failed
 * @param error exception
 */
void ion_defer_call(IONDeferCb *cb, zval *result, zval *error) {
    zval *helper = NULL;
    ALLOC_INIT_ZVAL(helper);
    if(result) {
        pionCbVoidWith3Args(cb->cb, result, helper, cb->arg);
    } else {
        pionCbVoidWith3Args(cb->cb, helper, error, cb->arg);
    }
    zval_ptr_dtor(&helper);
}

void ion_defer_cancel_call(IONDeferCb *cb, zval *cancel) {
    pionCbVoidWith2Args(cb->cb, cancel, cb->arg);
}

// Internals. Ctor and dtor.
static void _ion_defer_object_dtor(void *object TSRMLS_DC) {
    PHPDBG("Defer dtor");
    IONDefer *defer = (IONDefer *) object;
    zval* zgenerator;
    if(!(defer->flags & DEFER_FINISHED)) {
        ion_defer_cancel(defer, "The garbage collector has caused the destruction of the object due to the lack of references");
    }

    if(defer->flags & DEFER_USERSPACE) {
        DEFERCB_FREE(((IONDeferCb *)defer->obj));
    }

    if(defer->result) {
        zval_ptr_dtor(&defer->result);
    }
    if(defer->cb) {
        DEFERCB_FREE(defer->cb);
    }
//    if(defer->stack->head || defer->stack->tail) {
//        PHPDBG("stack not empty");
//    }
    if(defer->stack->head) {
        PHPDBG("Clean stack?");
        while(zgenerator = ion_llist_lpop(defer->stack)) {
            Z_DELREF_P(defer->stack);
        }
    }
    ion_llist_dtor(defer->stack);
    efree(defer);
}

static zend_object_value _ion_defer_object_ctor(zend_class_entry *ce TSRMLS_DC) {
IONDefer *defer = emalloc(sizeof(IONDefer));
memset(defer, 0, sizeof(IONDefer));
defer->stack = ion_llist_ctor();
OBJECT_INIT(retval, Defer, defer, _ion_defer_object_dtor);
return retval;
}

// C API
void _ion_php_cancel(zval *error, void *arg) {
IONDeferCb *cb = (IONDeferCb *)arg;
ion_defer_cancel_call(cb, error);

}

void _ion_defer_finish(short type, IONDefer *defer, zval *result TSRMLS_DC) {
    PHPDBG("finalize");
    if(defer->flags & DEFER_FINISHED) {
        IONF("Defer object already finished. Skip action.");
        return;
    }
    if(defer->result) {
        zval_ptr_dtor(&defer->result);
    }
    defer->result = result;
    Z_ADDREF_P(defer->result);
    IONF("Finalize defer object with status %d", (int)type);
    if(defer->cb) {
        if(type == DEFER_DONE) {
            ion_defer_call(defer->cb, result, NULL);
        } else {
            ion_defer_call(defer->cb, NULL, result);
        }
        DEFERCB_FREE(defer->cb);
        defer->cb = NULL;
    }

    if(defer->stack->head && defer->zself) {
        ion_enqueue(defer);
    }
    defer->flags |= type;
    if(defer->timeout) {
        event_del(defer->timeout);
        event_free(defer->timeout);
    }
//    if(defer->flags & DEFER_ALIVE) {
//        zval_ptr_dtor(&zdefer);
//    }
}

void _ion_defer_cancel(IONDefer *defer, char *msg TSRMLS_DC) {
    IONF("Cancellation defer object: %s", msg);
    zval *message = NULL;
    ALLOC_STRING_ZVAL(message, msg, 1);

    zval *result = pionNewObjectWith1Arg(CE(CancelException), message);
    zval_ptr_dtor(&message);
    defer->flags |= DEFER_CANCELED & DEFER_FAIL;
    if(defer->cancel_func) {
        defer->cancel_func(result, defer->obj);
    }
    ion_defer_fail(defer, result);
    zval_ptr_dtor(&result);
}

zval* _ion_defer_new(cancel_func fn, void *data) {
    zval *obj = NULL;
    ALLOC_INIT_ZVAL(obj);
    if(object_init_ex(obj, CE(Defer)) == FAILURE) {
        zval_ptr_dtor(&obj);
        return NULL;
    }

    IONDefer *defer = GET_OBJ_HANDLE(obj);
    if(fn) {
        defer->cancel_func = fn;
        defer->obj = data;
    }
    defer->result = NULL;
    defer->flags |= DEFER_INTERNAL;

    IONF("New internal defer-event was created.");

    return obj;
}

void _ion_defer_free(IONDefer *defer TSRMLS_DC) {
    CB_FREE(defer->cb);    \
    zval_ptr_dtor(&callback->arg); \
    efree(callback);
}

void ion_defer_set_cancel_cb(IONDefer *defer, cancel_func fn, void *data) {
    defer->cancel_func = fn;
    defer->obj = data;

    return;
}

// PHP API
/**
 * Defer::__construct(callable $cancel_cb = null, mixed $arg = null)
 */
PHP_METHOD(Defer, __construct) {
IONDefer *defer = FETCH_HANDLE();
zval *zarg = NULL;
IONDeferCb *dcb;
CB_INIT(cb);

if(defer->flags & (DEFER_INTERNAL | DEFER_FINISHED)) {
ThrowLogic("Failed to construct defer: defer is internal or already finished", -1);
}

PARSE_ARGS("|fz", CB_PARSE(cb), &zarg);

defer->cancel_func = _ion_php_cancel;
SETUP_ARG(zarg);
dcb = emalloc(sizeof(IONDeferCb));
dcb->cb = CB_NEW(cb);
dcb->arg = zarg;
defer->obj = (void *)dcb;
defer->flags |= DEFER_USERSPACE;

IONF("New defer object was created.");
}

ZEND_BEGIN_ARG_INFO_EX(arginfo___construct, 0, 0, 1)
ZEND_ARG_INFO(0, on_cancel_cb)
ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();

/**
 * Defer::onResult(callable $cancel_cb, mixed $arg = null)
 */
PHP_METHOD(Defer, onResult) {
IONDefer *defer = FETCH_HANDLE();
CB_INIT(cb);
zval *zarg = NULL;

PARSE_ARGS("f|z", CB_PARSE(cb), &zarg);

SETUP_ARG(zarg);

if(defer->flags & DEFER_FINISHED) {
IONF("Defer object already finished. Invoke callback immediately.");
// @todo
} else {
if(defer->cb) {
IONF("Cleanup previous onResult callback.");
DEFERCB_FREE(defer->cb);
}
IONF("Setup new onResult callback.");
defer->cb = emalloc(sizeof(IONDeferCb));
defer->cb->cb = CB_NEW(cb);
defer->cb->arg = zarg;
}
RETURN_THIS();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_onResult, 0, 0, 1)
ZEND_ARG_INFO(0, callback)
ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO();


PHP_METHOD(Defer, done) {
IONDefer *defer = FETCH_HANDLE();
zval *data;

if(defer->flags & DEFER_FINISHED) {
ThrowLogic("Failed to cancel finished defer-event", -1);
}
if(defer->flags & DEFER_INTERNAL) {
ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
}

PARSE_ARGS("z", &data);

ion_defer_done(defer, data);

RETURN_THIS();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_done, 0, 0, 1)
ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO();


PHP_METHOD(Defer, error) {
IONDefer *defer = FETCH_HANDLE();
zval *data;

if(defer->flags & DEFER_FINISHED) {
ThrowLogic("Failed to cancel finished defer-event", -1);
}
if(defer->flags & DEFER_INTERNAL) {
ThrowLogic("Internal defer-event cannot be finished by users", -1);
}

PARSE_ARGS("z", &data);

ion_defer_fail(defer, data);

RETURN_THIS();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_error, 0, 0, 1)
ZEND_ARG_OBJ_INFO(0, data, Exception, 0)
ZEND_END_ARG_INFO();
/* }}} */

/* {{{ */
PHP_METHOD(Defer, cancel) {
IONDefer *defer = FETCH_HANDLE();
char *message = NULL;
long message_len = 0;
if(defer->flags & DEFER_FINISHED) {
ThrowLogic("Failed to cancel finished defer-event", -1);
}
PARSE_ARGS("|s", &message, &message_len);

ion_defer_cancel(defer, (message_len ? message :"Canceled"));
RETURN_THIS();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_cancel, 0, 0, 0)
ZEND_END_ARG_INFO();
/* }}} */

/* {{{ */
static void _defer_timeout_callback(evutil_socket_t fd, short flags, void *arg) {
    zval *zdefer = (zval *)arg;
    ion_zdefer_cancel(zdefer, "Timed out");
}

PHP_METHOD(Defer, timeout) {
IONDefer *defer = FETCH_HANDLE();
long seconds = 0;

if(!(defer->flags & DEFER_FINISHED)) {
PARSE_ARGS("l", &seconds);
IONF("Set new timeout: %l sec", seconds);
struct timeval tv;
tv.tv_sec = seconds;
tv.tv_usec = seconds;

if(!defer->timeout) {
defer->timeout = make_event(-1, EV_TIMEOUT, _defer_timeout_callback, this_ptr);
}
event_add(defer->timeout, &tv);
}

RETURN_THIS();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_timeout, 0, 0, 1)
ZEND_ARG_INFO(0, seconds)
ZEND_END_ARG_INFO();
/* }}} */

PHP_METHOD(Defer, alive) {
IONDefer *defer = FETCH_HANDLE();
if(defer->flags & DEFER_FINISHED) {
ThrowLogic("Failed to cancel finished defer-event", -1);
}
if(!(defer->flags & DEFER_ALIVE)) {
zval_addref_p(this_ptr);
defer->flags |= DEFER_ALIVE;
}
RETURN_THIS();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_alive, 0, 0, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(Defer, dequeue) {

}

ZEND_BEGIN_ARG_INFO_EX(arginfo_dequeue, 0, 0, 0)
ZEND_END_ARG_INFO();



CLASS_METHODS(Defer)
ZEND_ME_ARG(Defer, __construct,    ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
ZEND_ME_ARG(Defer, onResult,       ZEND_ACC_PUBLIC)
ZEND_ME_ARG(Defer, done,           ZEND_ACC_PUBLIC)
ZEND_ME_ARG(Defer, error,          ZEND_ACC_PUBLIC)
ZEND_ME_ARG(Defer, cancel,         ZEND_ACC_PUBLIC)
ZEND_ME_ARG(Defer, timeout,        ZEND_ACC_PUBLIC)
ZEND_ME_ARG(Defer, alive,          ZEND_ACC_PUBLIC)
ZEND_ME_ARG(Defer, dequeue,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ion_defer) {
    REGISTER_CLASS(Defer, "Defer", _ion_defer_object_ctor);
    CE(Defer)->ce_flags |= ZEND_ACC_FINAL_CLASS;

    REGISTER_VOID_EXTENDED_CLASS(CancelException, Exception, "ION\\CancelException", NULL);
    REGISTER_VOID_EXTENDED_CLASS(TimeoutException, CancelException, "ION\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ion_defer) {
    zval *zdefer;
    while(zdefer = ion_llist_lpop(ION(queue))) {
        zval_ptr_dtor(&zdefer);
    }
}