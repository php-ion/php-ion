#include "ResolvablePromise.h"
#include "Promise.h"

ION_DEFINE_CLASS(ION_ResolvablePromise);


/** public function ION\ResolvablePromise::done(mixed $data) : self */
CLASS_METHOD(ION_ResolvablePromise, done) {
    ion_promise * promise = getThisInstance();
    zval *data = NULL;
    if(promise->flags & ION_DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished defer-event", -1);
        return;
    }
    if(promise->flags & ION_DEFERRED_INTERNAL) {
        ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
    }
    PARSE_ARGS("z", &data);
    ion_promise_done(getThis(), data);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_ResolvablePromise, done, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()

/** public function ION\ResolvablePromise::fail(Exception $error) : self */
CLASS_METHOD(ION_ResolvablePromise, fail) {
    ion_promise * promise = getThisInstance();
    zval *error;
    if(promise->flags & ION_DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished defer-event", -1);
        return;
    }
    if(promise->flags & ION_DEFERRED_INTERNAL) {
        ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
        return;
    }
    PARSE_ARGS("z", &error);
    ion_promise_fail(getThis(), error);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_ResolvablePromise, fail, 1)
    METHOD_ARG_OBJECT(data, Exception, 0, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_ResolvablePromise)
    METHOD(ION_ResolvablePromise, done,          ZEND_ACC_PUBLIC)
    METHOD(ION_ResolvablePromise, fail,          ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_ResolvablePromise) {
    REGISTER_EXTENDED_CLASS(ION_ResolvablePromise, ION_Promise, "ION\\ResolvablePromise");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_ResolvablePromise) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_ResolvablePromise) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_ResolvablePromise) {
    return SUCCESS;
}