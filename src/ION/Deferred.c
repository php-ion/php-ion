#include "Deferred.h"

#define CB_INIT(cb)                                           \
    zend_fcall_info        cb ## _fci = empty_fcall_info;       \
    zend_fcall_info_cache  cb ## _fcc = empty_fcall_info_cache;

#define getInstance(zobj)   zend_object_store_get_object(zobj TSRMLS_CC)


#define PION_CLASS_CONST_LONG(class, const_name, value) \
    zend_declare_class_constant_long(c ## class, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);

inline void ionDeferredFinish(zval *zDeferred, zval *result, short type) {
    IONDeferred *deferred = getInstance(zDeferred);
}

inline void ionDeferredReject(zval *zDeferred, char *message) {
    IONDeferred *deferred = getInstance(zDeferred);
    IONF("Cancellation defer object: %s", message);
    zval *zMessage = NULL;
    ALLOC_STRING_ZVAL(zMessage, message, 1);
    zval *result = pionNewObjectWith1Arg(CE(ION_Deferred_RejectException), zMessage);
    zval_ptr_dtor(&zMessage);
    deferred->flags |= DEFERRED_REJECTED;
    if(deferred->deferredCancelFunc) {
        deferred->deferredCancelFunc(result, deferred);
    }
    ionDeferredFinish(zDeferred, result, DEFERRED_FAILED);
}

//inline void iodDeferredStore()

static void _ionRejectFromPHP(zval *error, IONDeferred *deferred) {
//    IONDeferred *deferred = (IONDeferred *)deferred;
//    if(deferred->cancel_cb) {
        pionCbVoidWith1Arg(deferred->cancel_cb, deferred->result);
//    }
//    IONDeferCb *cb = (IONDeferCb *)arg;
//    ion_defer_cancel_call(cb, error);
//    DEFERCB_FREE(cb);
}

CLASS_INSTANCE_DTOR(ION_Deferred) {
    IONDeferred *defer = getInstanceObject(IONDeferred *);
    efree(defer);
}

CLASS_INSTANCE_CTOR(ION_Deferred) {
    IONDeferred *object = emalloc(sizeof(IONDeferred));
    memset(object, 0, sizeof(IONDeferred));

    RETURN_INSTANCE(ION_Deferred, object);
}

/** public function ION\Deferred::__construct(callable $cancel_callback) : self */
CLASS_METHOD(ION_Deferred, __construct) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);
    zval *zCallback = NULL;
    PARSE_ARGS("z", &zCallback);
    deferred->deferredCancelFunc = (deferredCancelFunc) _ionRejectFromPHP;
    deferred->cancel_cb = pionCbCreateFromZval(zCallback);
}

METHOD_ARGS_BEGIN(ION_Deferred, __construct, 1)
    METHOD_ARG_TYPE(cancel_callback, IS_CALLABLE, 0, 0)
METHOD_ARGS_END();

/** public function ION\Deferred::then(callable $callback) : self */
CLASS_METHOD(ION_Deferred, then) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);
    zval *zCallback = NULL;
    PARSE_ARGS("z", &zCallback);
    deferred->finish_cb = pionCbCreateFromZval(zCallback);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, then, 1)
    METHOD_ARG_TYPE(callback, IS_CALLABLE, 0, 0)
METHOD_ARGS_END();

/** public function ION\Deferred::reject(string $reason) : self */
CLASS_METHOD(ION_Deferred, reject) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);
    char *message = NULL;
    long message_len = 0;
    if(deferred->flags & DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished deferred object", -1);
        return;
    }

    PARSE_ARGS("s", &message, &message_len);
    ionDeferredReject(getThis(), message);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, reject, 1)
    METHOD_ARG_TYPE(reason, IS_STRING, 0, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::resolve(mixed $data) : self */
CLASS_METHOD(ION_Deferred, resolve) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);
    zval *zData;
    if(deferred->scope) {
        if(deferred->scope != EG(scope)) {
            ThrowLogic("Invalid call scope", -1);
            return;
        }
    }
    if(deferred->flags & DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished defer-event", -1);
        return;
    }
    if(deferred->flags & DEFERRED_INTERNAL) {
        ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
    }
    PARSE_ARGS("z", &zData);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, resolve, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::resolve(mixed $data) : self */
CLASS_METHOD(ION_Deferred, error) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, error, 1)
    METHOD_ARG_OBJECT(data, "Exception", 0, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::timeout(int $seconds) : self */
CLASS_METHOD(ION_Deferred, timeout) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, timeout, 1)
    METHOD_ARG_TYPE(data, IS_LONG, 0, 0)
METHOD_ARGS_END()

/** public function ION\Deferred::getFlags() : int */
CLASS_METHOD(ION_Deferred, getFlags) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);

    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Deferred, getFlags)


CLASS_METHODS_START(ION_Deferred)
    METHOD(ION_Deferred, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, then, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, reject, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, resolve, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, error, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, timeout, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, getFlags, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Deferred) {
    PION_REGISTER_CLASS(ION_Deferred, "ION\\Deferred");
    CE(ION_Deferred)->ce_flags |= ZEND_ACC_FINAL_CLASS;
    PION_CLASS_CONST_LONG(ION_Deferred, "RESOLVED",  DEFERRED_RESOLVED);
    PION_CLASS_CONST_LONG(ION_Deferred, "FAILED",    DEFERRED_FAILED);
    PION_CLASS_CONST_LONG(ION_Deferred, "FINISHED",  DEFERRED_FINISHED);
    PION_CLASS_CONST_LONG(ION_Deferred, "INTERNAL",  DEFERRED_INTERNAL);
    PION_CLASS_CONST_LONG(ION_Deferred, "TIMED_OUT", DEFERRED_TIMED_OUT);
    PION_CLASS_CONST_LONG(ION_Deferred, "REJECTED",  DEFERRED_REJECTED);

    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_RejectException, Exception, "ION\\Deferred\\RejectException", NULL);
    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_TimeoutException, ION_Deferred_RejectException, "ION\\Deferred\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Deferred) {
    return SUCCESS;
}