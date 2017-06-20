#include "ion.h"

zend_class_entry * ion_ce_ION_ResolvablePromise;
zend_object_handlers ion_oh_ION_ResolvablePromise;


/** public function ION\ResolvablePromise::done(mixed $data) : self */
CLASS_METHOD(ION_ResolvablePromise, done) {
    ion_promisor * promise = ION_THIS_OBJECT(ion_promisor);
    zval * data = NULL;
    if(promise->flags & ION_PROMISOR_FINISHED) {
        zend_throw_error(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_ALREADY_FINISHED);
        return;
    }
    if(promise->flags & ION_PROMISOR_INTERNAL) {
        zend_throw_error(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_FINISH_INTERNAL);
        return;
    }
    if(promise->await || promise->generator) {
        zend_throw_error(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_YIELDED);
        return;
    }
    if(promise->scope) {
        // todo ...
    }
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(data)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_promisor_done(promise, data);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_ResolvablePromise, done, 1)
    ARGUMENT(data, IS_MIXED)
METHOD_ARGS_END()

/** public function ION\ResolvablePromise::fail(Throwable $error) : self */
CLASS_METHOD(ION_ResolvablePromise, fail) {
    ion_promisor * promise = ION_THIS_OBJECT(ion_promisor);
    zval * error = NULL;
    if(promise->flags & ION_PROMISOR_FINISHED) {
        zend_throw_error(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_ALREADY_FINISHED);
        return;
    }
    if(promise->flags & ION_PROMISOR_INTERNAL) {
        zend_throw_error(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_FINISH_INTERNAL);
        return;
    }
    if(promise->await || promise->generator) {
        zend_throw_error(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_YIELDED);
        return;
    }
    if(promise->scope) {
        // todo ...
    }
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(error)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_promisor_fail(promise, error);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_ResolvablePromise, fail, 1)
    ARGUMENT_OBJECT(data, Throwable, 0)
METHOD_ARGS_END()

METHODS_START(methods_ION_ResolvablePromise)
    METHOD(ION_ResolvablePromise, done, ZEND_ACC_PUBLIC)
    METHOD(ION_ResolvablePromise, fail, ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_ResolvablePromise) {
    ion_register_class_ex(&ion_ce_ION_ResolvablePromise, ion_ce_ION_Promise, "ION\\ResolvablePromise", ion_promise_zend_init, methods_ION_ResolvablePromise);
    ion_init_object_handlers(ion_oh_ION_ResolvablePromise);
    ion_oh_ION_ResolvablePromise.free_obj = ion_promisor_zend_free;
    ion_oh_ION_ResolvablePromise.clone_obj = ion_promisor_zend_clone;
    ion_oh_ION_ResolvablePromise.offset = ion_offset(ion_promisor);
    return SUCCESS;
}