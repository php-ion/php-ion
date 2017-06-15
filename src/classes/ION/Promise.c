#include "ion.h"

zend_object_handlers ion_oh_ION_Promise;
zend_class_entry * ion_ce_ION_Promise;

zend_object * ion_promise_zend_init(zend_class_entry * ce) {
    ion_promisor * promise = ion_alloc_object(ce, ion_promisor);
    promise->flags |= ION_PROMISOR_TYPE_PROMISE;
    ZVAL_UNDEF(&promise->object);
    return ion_init_object(ION_OBJECT_ZOBJ(promise), ce, &ion_oh_ION_Promise);
}

/** public function ION\Promise::__construct(callable $done = null, callable $fail = null) : int */
CLASS_METHOD(ION_Promise, __construct) {
    zval * done = NULL;
    zval * fail = NULL;
    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(done, 1, 0)
        Z_PARAM_ZVAL_EX(fail, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_promisor_set_callbacks(ION_THIS_OBJECT(ion_promisor), done, fail);
}

METHOD_ARGS_BEGIN(ION_Promise, __construct, 0)
    METHOD_ARG_CALLBACK(done, 0, 1)
    METHOD_ARG_CALLBACK(fail, 0, 1)
METHOD_ARGS_END();

/** public function ION\Promise::then(callable $done = null, callable $fail = null) : ION\Promise */
/** public function ION\Promise::then(ION\Promise $handler) : ION\Promise */
CLASS_METHOD(ION_Promise, then) {
    ion_promisor * handler = NULL;
    zval        * done = NULL;
    zval        * fail = NULL;
    ion_promisor * promisor = ION_THIS_OBJECT(ion_promisor);

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(done, 1, 0)
        Z_PARAM_ZVAL_EX(fail, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(done && Z_ISPROMISE_P(done)) {
        if(Z_OBJ_P(done) == Z_OBJ_P(getThis())) {
            zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_PROMISE_ITSELF, 0);
            return;
        }
        handler = ION_ZVAL_OBJECT_P(done, ion_promisor);
        ion_promisor_append(promisor, handler);
        zval_add_ref(done);
        RETURN_OBJ(ION_OBJECT_ZOBJ(handler));
    } else {
        handler = ion_promisor_push_callbacks(promisor, done, fail);
        if(handler == NULL) {
            if(!EG(exception)) {
                zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_PROMISE_CANT, 0);
            }
            return;
        }
    }
    RETURN_ION_OBJ(handler);
}

METHOD_ARGS_BEGIN(ION_Promise, then, 0)
    METHOD_ARG(done, 0)
    METHOD_ARG_CALLBACK(fail, 0, 1)
METHOD_ARGS_END();

CLASS_METHOD(ION_Promise, forget) {
    zval * handler = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(handler)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(Z_ISPROMISE_P(handler)) {
        ion_promisor_remove(ION_THIS_OBJECT(ion_promisor), ION_ZVAL_OBJECT_P(handler, ion_promisor));
    } else if(Z_TYPE_P(handler) == IS_STRING) {
        ion_promisor_remove_named(ION_THIS_OBJECT(ion_promisor), Z_STR_P(handler));
    } else {
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_PROMISE_ONLY_PROMISES, 0);
        return;
    }
}

METHOD_ARGS_BEGIN(ION_Promise, forget, 0)
    METHOD_ARG(handler, 0)
METHOD_ARGS_END();

/** public function ION\Promise::onDone(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onDone) {
    zval * callback = NULL;
    ion_promisor * promise = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    promise = ion_promisor_push_callbacks(ION_THIS_OBJECT(ion_promisor), callback, NULL);
    if(promise == NULL) {
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_PROMISE_CANT, 0);
        return;
    }
    RETURN_OBJ(ION_OBJECT_ZOBJ(promise));
}

METHOD_ARGS_BEGIN(ION_Promise, onDone, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::onFail(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onFail) {
    zval * callback = NULL;
    ion_promisor * promise = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    promise = ion_promisor_push_callbacks(ION_THIS_OBJECT(ion_promisor), NULL, callback);
    if(promise == NULL) {
        zend_throw_exception(ion_class_entry(InvalidArgumentException), ERR_ION_PROMISE_CANT, 0);
        return;
    }
    RETURN_OBJ(ION_OBJECT_ZOBJ(promise));
}

METHOD_ARGS_BEGIN(ION_Promise, onFail, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::getState() : string */
CLASS_METHOD(ION_Promise, getState) {
    ion_promisor * promise = ION_THIS_OBJECT(ion_promisor);
    zend_string * state;
    if(promise->flags & ION_PROMISOR_DONE) {
        state = ION_STR(ION_STR_DONE);
    } else if(promise->flags & ION_PROMISOR_CANCELED) {
        state = ION_STR(ION_STR_CANCELED);
    } else if(promise->flags & ION_PROMISOR_FAILED) {
        state = ION_STR(ION_STR_FAILED);
    } else if(promise->flags & ION_PROMISOR_PROCESSING) {
        state = ION_STR(ION_STR_IN_PROGRESS);
    } else {
        state = ION_STR(ION_STR_PENDING);
    }

    RETURN_STR(state);
}

METHOD_WITHOUT_ARGS(ION_Promise, getState)

/** public function ION\Promise::getFlags() : string */
CLASS_METHOD(ION_Promise, getFlags) {
    ion_promisor * promise = ION_THIS_OBJECT(ion_promisor);
    RETURN_LONG(promise->flags);
}

METHOD_WITHOUT_ARGS(ION_Promise, getFlags)

/** public function ION\Promise::setUID(int $uid) : self */
CLASS_METHOD(ION_Promise, setName) {
    ion_promisor * promise = ION_THIS_OBJECT(ion_promisor);
    zend_string  * name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
    promise->name = zend_string_copy(name);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Promise, setName, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END()

CLASS_METHODS_START(ION_Promise)
    METHOD(ION_Promise, __construct,   ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, then,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, forget,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onDone,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onFail,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, getState,      ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, getFlags,      ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, setName,       ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Promise) {
    pion_register_class(ION_Promise, "ION\\Promise", ion_promise_zend_init, CLASS_METHODS(ION_Promise));
    pion_init_std_object_handlers(ION_Promise);
    pion_set_object_handler(ION_Promise, free_obj, ion_promisor_zend_free);
    pion_set_object_handler(ION_Promise, clone_obj, ion_promisor_zend_clone);
    ion_class_set_offset(ion_oh_ION_Promise, ion_promisor);


    PION_CLASS_CONST_LONG(ION_Promise, "DONE",      ION_PROMISOR_DONE);
    PION_CLASS_CONST_LONG(ION_Promise, "FAILED",    ION_PROMISOR_FAILED);
    PION_CLASS_CONST_LONG(ION_Promise, "FINISHED",  ION_PROMISOR_FINISHED);
    PION_CLASS_CONST_LONG(ION_Promise, "INTERNAL",  ION_PROMISOR_INTERNAL);
    PION_CLASS_CONST_LONG(ION_Promise, "TIMED_OUT", ION_PROMISOR_TIMED_OUT);
    PION_CLASS_CONST_LONG(ION_Promise, "CANCELED",  ION_PROMISOR_CANCELED);
    return SUCCESS;
}
