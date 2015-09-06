#include "Deferred.h"
#include "../pion/linkedlist.h"

#define CALL_OBJECT_DTOR(deferred, zDeferred)                          \
    if(deferred->object && deferred->object_dtor) {                    \
        deferred->object_dtor(deferred->object, zDeferred TSRMLS_CC);  \
        deferred->object = NULL;                                       \
    }

#define CLEAN_DEFERRED(deferred)                \
    if(deferred->cancel_cb) {                   \
        pionCbFree(deferred->cancel_cb);        \
        deferred->cancel_cb = NULL;             \
    }                                           \
    if(deferred->finish_cb) {                   \
        pionCbFree(deferred->finish_cb);        \
        deferred->finish_cb = NULL;             \
    }

zval *_ion_deferred_new(zval *zCancelCb TSRMLS_DC) {
    zval *zDeferred = NULL;
    if(zCancelCb) {
        zDeferred = pionNewObjectWith1Arg(CE(ION_Deferred), zCancelCb TSRMLS_CC);
    } else {
        zDeferred = pionNewObject(CE(ION_Deferred), 0, NULL TSRMLS_CC);
    }
    return zDeferred;
}

zval *_ion_deferred_new_ex(deferred_reject_callback cancel_cb TSRMLS_DC) {
    zval *zDeferred = NULL;
    ALLOC_INIT_ZVAL(zDeferred);
    object_init_ex(zDeferred, CE(ION_Deferred));
    ion_deferred *deferred = getInstance(zDeferred);
    deferred->reject = cancel_cb;
    deferred->flags |= ION_DEFERRED_INTERNAL;
    return zDeferred;
}

int _ion_deferred_zval(zval *zvariable, void *object, deferred_object_dtor dtor, deferred_reject_callback reject_cb TSRMLS_DC) {
    object_init_ex(zvariable, CE(ION_Deferred));
    ion_deferred *deferred = getInstance(zvariable);
    deferred->flags |= ION_DEFERRED_INTERNAL;
    deferred->reject = reject_cb;
    deferred->object = object;
    deferred->object_dtor = dtor;
    return SUCCESS;
}


void _ion_deferred_free(zval *zDeferred TSRMLS_DC) {
    ion_deferred *deferred = getInstance(zDeferred);
    CLEAN_DEFERRED(deferred);
}

void _ion_deferred_store(zval *zDeferred, void *object, deferred_object_dtor dtor TSRMLS_DC) {
    ion_deferred *deferred = getInstance(zDeferred);
    if(deferred->object && deferred->object_dtor) {
        IONF("Cleanup prevoiuse stored object");
        deferred->object_dtor(deferred->object, zDeferred TSRMLS_CC);
    }
    deferred->object = object;
    deferred->object_dtor = dtor;
}

void *_ion_deferred_store_get(zval *zDeferred TSRMLS_DC) {
    ion_deferred *deferred = getInstance(zDeferred);
    return deferred->object;
}

void _ion_deferred_resolve(zval *zDeferred, zval * zresult, short type TSRMLS_DC) {
    ion_deferred *deferred = getInstance(zDeferred);
    int result = 0;
    deferred->flags |= type;
    deferred->result = zresult;
    zval_add_ref(&zresult);
    if(deferred->finish_cb) {
        zval * helper = NULL;
        ALLOC_INIT_ZVAL(helper);
        if(type == ION_DEFERRED_DONE) {
            result = pionCbVoidWith2Args(deferred->finish_cb, zresult, helper TSRMLS_CC);
        } else {
            result = pionCbVoidWith2Args(deferred->finish_cb, helper, zresult TSRMLS_CC);
        }
        zval_ptr_dtor(&helper);
        if(result == FAILURE) {
            PHPDBG("ION: deferred callback corrupted");
        }
    }

    CLEAN_DEFERRED(deferred);
    CALL_OBJECT_DTOR(deferred, zDeferred);
}

void _ion_deferred_done_long(zval *zdeferred, long * lval TSRMLS_DC) {
    zval * zlong = NULL;
    ALLOC_LONG_ZVAL(zlong, lval);
    ion_deferred_done(zdeferred, zlong);
    zval_ptr_dtor(&zlong);
}

void _ion_deferred_done_bool(zval *zdeferred, zend_bool bval TSRMLS_DC) {
    zval * zbool = NULL;
    ALLOC_BOOL_ZVAL(zbool, bval);
    ion_deferred_done(zdeferred, zbool);
    zval_ptr_dtor(&zbool);
}

void _ion_deferred_done_stringl(zval *zdeferred, char * str, long length, int dup TSRMLS_DC) {
    zval * zstring = NULL;
    ALLOC_STRINGL_ZVAL(zstring, str, length, dup);
    ion_deferred_done(zdeferred, zstring);
    zval_ptr_dtor(&zstring);
}

void _ion_deferred_exception_ex(zval * zdeferred, zend_class_entry * ce, long code TSRMLS_DC, const char * message, ...) {
    va_list args;
    zval * zexception = NULL;
    va_start(args, message);
    zexception = pion_exception_new_ex(ce, code, message, args);
    va_end(args);
    ion_deferred_fail(zdeferred, zexception);
    zval_ptr_dtor(&zexception);
}

void _ion_deferred_reject(zval *zDeferred, const char *message TSRMLS_DC) {
    ion_deferred * deferred = getInstance(zDeferred);
    IONF("Cancellation defer object: %s", message);
    zval *zException = _pion_exception_new(CE(ION_Deferred_RejectException), message, 0 TSRMLS_CC);
    deferred->flags |= ION_DEFERRED_REJECTED | ION_DEFERRED_FAILED;
    if(deferred->reject) {
        deferred->reject(zException, zDeferred TSRMLS_CC);
    }
    CLEAN_DEFERRED(deferred);
    CALL_OBJECT_DTOR(deferred, zDeferred);
}


static void _ion_deferred_reject_php(zval * error, zval * zdeferred TSRMLS_DC) {
    ion_deferred * deferred = getInstance(zdeferred);
    pionCbVoidWith1Arg(deferred->cancel_cb, error TSRMLS_CC);
    zval_ptr_dtor(&error);
}

CLASS_INSTANCE_DTOR(ION_Deferred) {
    ion_deferred *deferred = getInstanceObject(ion_deferred *);
    CLEAN_DEFERRED(deferred);
    efree(deferred);
}

CLASS_INSTANCE_CTOR(ION_Deferred) {
    ion_deferred *object = emalloc(sizeof(ion_deferred));
    memset(object, 0, sizeof(ion_deferred));

    RETURN_INSTANCE(ION_Deferred, object);
}

/** public function ION\Deferred::__construct(callable $cancel_callback) : self */
CLASS_METHOD(ION_Deferred, __construct) {
    ion_deferred *deferred = getThisInstance();
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("f", &fci, &fcc);
    deferred->reject = (deferred_reject_callback) _ion_deferred_reject_php;
    deferred->cancel_cb = pionCbCreate(&fci, &fcc TSRMLS_CC);
}

METHOD_ARGS_BEGIN(ION_Deferred, __construct, 1)
    METHOD_ARG_TYPE(cancel_callback, IS_CALLABLE, 0, 0)
METHOD_ARGS_END();

/** public function ION\Deferred::then(callable $callback) : self */
CLASS_METHOD(ION_Deferred, then) {
    ion_deferred *deferred = getThisInstance();
    zval * znull;
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("f", &fci, &fcc);

    if(deferred->result) {
        ALLOC_INIT_ZVAL(znull);
        if(deferred->flags & ION_DEFERRED_DONE) {
            pion_fcall_void_2_args(&fci, &fcc, deferred->result, znull);
        } else {
            pion_fcall_void_2_args(&fci, &fcc, znull, deferred->result);
        }
        zval_ptr_dtor(&znull);
    } else {
        deferred->finish_cb = pionCbCreate(&fci, &fcc TSRMLS_CC);
    }
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, then, 1)
    METHOD_ARG_TYPE(callback, IS_CALLABLE, 0, 0)
METHOD_ARGS_END();

/** public function ION\Deferred::reject(string $reason) : self */
CLASS_METHOD(ION_Deferred, reject) {
    ion_deferred *deferred = getThisInstance();
    char *message = NULL;
    long message_len = 0;
    if(deferred->flags & ION_DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished deferred object", -1);
        return;
    }

    PARSE_ARGS("s", &message, &message_len);
    _ion_deferred_reject(getThis(), message TSRMLS_CC);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, reject, 1)
    METHOD_ARG(reason, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::resolve(mixed $data) : self */
CLASS_METHOD(ION_Deferred, resolve) {
    ion_deferred *deferred = getThisInstance();
    zval *zData = NULL;
    if(deferred->scope) {
        if(deferred->scope != EG(scope)) {
            ThrowLogic("Invalid call scope", -1);
            return;
        }
    }
    if(deferred->flags & ION_DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished defer-event", -1);
        return;
    }
    if(deferred->flags & ION_DEFERRED_INTERNAL) {
        ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
    }
    PARSE_ARGS("z", &zData);
    _ion_deferred_resolve(getThis(), zData, ION_DEFERRED_DONE TSRMLS_CC);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, resolve, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::error(Exception $error) : self */
CLASS_METHOD(ION_Deferred, error) {
    ion_deferred *deferred = getThisInstance();
    zval *zError;
    if(deferred->scope) {
        if(deferred->scope != EG(scope)) {
            ThrowLogic("Invalid call scope", -1);
            return;
        }
    }
    if(deferred->flags & ION_DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished defer-event", -1);
        return;
    }
    if(deferred->flags & ION_DEFERRED_INTERNAL) {
        ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
        return;
    }
    PARSE_ARGS("z", &zError);
    _ion_deferred_resolve(getThis(), zError, ION_DEFERRED_FAILED TSRMLS_CC);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, error, 1)
    METHOD_ARG_OBJECT(data, Exception, 0, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::timeout(int $seconds) : self */
CLASS_METHOD(ION_Deferred, timeout) {
//    ion_deferred *deferred = getThisInstance();

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, timeout, 1)
    METHOD_ARG_TYPE(data, IS_LONG, 0, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::getFlags() : int */
CLASS_METHOD(ION_Deferred, getFlags) {
    ion_deferred *deferred = getThisInstance();
    RETURN_LONG((long)deferred->flags);
}

METHOD_WITHOUT_ARGS(ION_Deferred, getFlags)

/** public function ION\Deferred::__destruct() : int */
CLASS_METHOD(ION_Deferred, __destruct) {
//    PHPDBG("deferred destructed")
//    zend_error(E_USER_NOTICE, "Deferred destruct");
    ion_deferred *deferred = getThisInstance();
    if(deferred->result) {
        zval_ptr_dtor(&deferred->result);
        deferred->result = NULL;
    }
    if(deferred->flags & ION_DEFERRED_FINISHED) {
        return;
    } else {
        ion_deferred_reject(getThis(), "Object destruct");
    }
}

METHOD_WITHOUT_ARGS(ION_Deferred, __destruct)


CLASS_METHODS_START(ION_Deferred)
    METHOD(ION_Deferred, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, then, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, reject, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, resolve, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, error, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, timeout, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, getFlags, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, __destruct, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Deferred) {
    PION_REGISTER_CLASS(ION_Deferred, "ION\\Deferred");
    CE(ION_Deferred)->ce_flags |= ZEND_ACC_FINAL_CLASS;
    PION_CLASS_CONST_LONG(ION_Deferred, "RESOLVED", ION_DEFERRED_DONE);
    PION_CLASS_CONST_LONG(ION_Deferred, "FAILED", ION_DEFERRED_FAILED);
    PION_CLASS_CONST_LONG(ION_Deferred, "FINISHED", ION_DEFERRED_FINISHED);
    PION_CLASS_CONST_LONG(ION_Deferred, "INTERNAL", ION_DEFERRED_INTERNAL);
    PION_CLASS_CONST_LONG(ION_Deferred, "TIMED_OUT", ION_DEFERRED_TIMED_OUT);
    PION_CLASS_CONST_LONG(ION_Deferred, "REJECTED", ION_DEFERRED_REJECTED);

    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_RejectException, Exception, "ION\\Deferred\\RejectException", NULL);
    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_TimeoutException, ION_Deferred_RejectException, "ION\\Deferred\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Deferred) {
    return SUCCESS;
}