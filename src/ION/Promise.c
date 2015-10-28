#include "../pion.h"
//#include "Deferred.h"
//#include <Zend/zend_generators.h>

zend_object_handlers ion_oh_ION_Promise;
zend_class_entry * ion_ce_ION_Promise;


zend_object * ion_promise_init(zend_class_entry * ce) {
    ion_promisor * promise = ecalloc(1, sizeof(ion_promisor));
//    ion_promisor * promise = ecalloc(1, sizeof(ion_promisor));
//    memset(promise, 0, sizeof(ion_promisor));
    promise->flags |= ION_PROMISOR_TYPE_PROMISE;
    RETURN_INSTANCE(ION_Promise, promise);
}

/** public function ION\Promise::__construct(callable $done = null, callable $fail = null, callable $progress = null) : int */
/** public function ION\Promise::__construct(callable $done = null, callable $progress = null) : int */
CLASS_METHOD(ION_Promise, __construct) {
    zval * done = NULL;
    zval * fail = NULL;
    zval * progress = NULL;
    ZEND_PARSE_PARAMETERS_START(0, 3)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(done, 1, 0)
        Z_PARAM_ZVAL_EX(fail, 1, 0)
        Z_PARAM_ZVAL_EX(progress, 1, 0)
    ZEND_PARSE_PARAMETERS_END();
    ion_promisor_set_callbacks(Z_OBJ_P(getThis()), done, fail, progress);
}

METHOD_ARGS_BEGIN(ION_Promise, __construct, 0)
    METHOD_ARG_CALLBACK(done, 0, 1)
    METHOD_ARG_CALLBACK(fail, 0, 1)
    METHOD_ARG_CALLBACK(progress, 0, 1)
METHOD_ARGS_END();

/** public function ION\Promise::then(callable $done = null, callable $fail = null, callable $progress = null) : ION\Promise */
CLASS_METHOD(ION_Promise, then) {
    zend_object * promise = NULL;
    zval        * done = NULL;
    zval        * fail = NULL;
    zval        * progress = NULL;
    ZEND_PARSE_PARAMETERS_START(0, 3)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(done, 1, 0)
        Z_PARAM_ZVAL_EX(fail, 1, 0)
        Z_PARAM_ZVAL_EX(progress, 1, 0)
    ZEND_PARSE_PARAMETERS_END();
    promise = ion_promisor_push_callbacks(Z_OBJ_P(getThis()), done, fail, progress);
    if(promise == NULL) {
        zend_throw_error(ion_class_entry(InvalidArgumentException), "Can't promise");
        return;
    }

    RETURN_OBJ_ADDREF(promise);
}

METHOD_ARGS_BEGIN(ION_Promise, then, 0)
    METHOD_ARG_CALLBACK(done, 0, 1)
    METHOD_ARG_CALLBACK(fail, 0, 1)
    METHOD_ARG_CALLBACK(progress, 0, 1)
METHOD_ARGS_END();

/** public function ION\Promise::onDone(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onDone) {
    zval * callback = NULL;
    zend_object * promise = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END();
    promise = ion_promisor_push_callbacks(Z_OBJ_P(getThis()), callback, NULL, NULL);
    if(promise == NULL) {
        zend_throw_error(ion_class_entry(InvalidArgumentException), "Can't promise");
        return;
    }
    RETURN_OBJ_ADDREF(promise);
}

METHOD_ARGS_BEGIN(ION_Promise, onDone, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::onFail(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onFail) {
    zval * callback = NULL;
    zend_object * promise = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END();
    promise = ion_promisor_push_callbacks(Z_OBJ_P(getThis()), NULL, callback, NULL);
    if(promise == NULL) {
        zend_throw_error(ion_class_entry(InvalidArgumentException), "Can't promise");
        return;
    }
    RETURN_OBJ_ADDREF(promise);
}

METHOD_ARGS_BEGIN(ION_Promise, onFail, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::onProgress(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onProgress) {
    zval * callback = NULL;
    zend_object * promise = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END();
    promise = ion_promisor_push_callbacks(Z_OBJ_P(getThis()), NULL, NULL, callback);
    if(promise == NULL) {
        zend_throw_error(ion_class_entry(InvalidArgumentException), "Can't promise");
        return;
    }
    RETURN_OBJ_ADDREF(promise);
}

METHOD_ARGS_BEGIN(ION_Promise, onProgress, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::getState() : string */
CLASS_METHOD(ION_Promise, getState) {
    ion_promisor * promise = get_this_instance(ion_promisor);
    zend_string * state;
    if(promise->flags & ION_PROMISOR_DONE) {
        state = zend_string_init("done", strlen("done"), 0);
    } else if(promise->flags & ION_PROMISOR_CANCELED) {
        state = zend_string_init("canceled", strlen("canceled"), 0);
    } else if(promise->flags & ION_PROMISOR_FAILED) {
        state = zend_string_init("failed", strlen("failed"), 0);
    } else if(promise->await || promise->generator) {
        state = zend_string_init("in_work", strlen("in_work"), 0);
    } else {
        state = zend_string_init("pending", strlen("pending"), 0);
    }

    RETURN_STR(state);
}

METHOD_WITHOUT_ARGS_RETURN_STRING(ION_Promise, getState)

/** public function ION\Promise::getFlags() : string */
CLASS_METHOD(ION_Promise, getFlags) {
    ion_promisor * promise = get_this_instance(ion_promisor);
    RETURN_LONG(promise->flags);
}

METHOD_WITHOUT_ARGS_RETURN_INT(ION_Promise, getFlags)

/** public function ION\Promise::__destruct() : int */
CLASS_METHOD(ION_Promise, __destruct) {
//    ion_promisor * promise = get_this_instance(ion_promisor);
//    PHPDBG("Clean promise %d", (int)promise->uid);
//    if(promise->done) {
//        pion_cb_free(promise->done);
//        promise->done = NULL;
//    }
//    if(promise->fail) {
//        pion_cb_free(promise->fail);
//        promise->fail = NULL;
//    }
//    if(promise->progress) {
//        pion_cb_free(promise->progress);
//        promise->progress = NULL;
//    }
//    if(promise->handler_count) {
//        for(uint i=0; i<promise->handler_count; i++) {
//            obj_ptr_dtor(promise->handlers[i]);
//        }
//        efree(promise->handlers);
//        promise->handler_count = 0;
//    }
//    zval_ptr_dtor(&promise->result);
}

METHOD_WITHOUT_ARGS(ION_Promise, __destruct)

#ifdef ION_DEBUG

/** public function ION\Promise::setUID(int $uid) : self */
CLASS_METHOD(ION_Promise, setUID) {
    ion_promisor * promise = get_this_instance(ion_promisor);
    long uid = 0;
    PARSE_ARGS("l", &uid);
    promise->uid = uid;
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Promise, setUID, 1)
    METHOD_ARG(uid, 0)
METHOD_ARGS_END()

#endif


CLASS_METHODS_START(ION_Promise)
    METHOD(ION_Promise, __construct,   ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, __destruct,    ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, then,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onDone,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onFail,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onProgress,    ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, getState,      ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, getFlags,      ZEND_ACC_PUBLIC)
#ifdef ION_DEBUG
    METHOD(ION_Promise, setUID,        ZEND_ACC_PUBLIC)
#endif
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Promise) {
    pion_register_class(ION_Promise, "ION\\Promise", ion_promise_init, CLASS_METHODS(ION_Promise));
    pion_init_std_object_handlers(ION_Promise);
    pion_set_object_handler(ION_Promise, free_obj, ion_promisor_free);
    pion_set_object_handler(ION_Promise, clone_obj, ion_promisor_clone_obj);

    PION_CLASS_CONST_LONG(ION_Promise, "DONE",      ION_PROMISOR_DONE);
    PION_CLASS_CONST_LONG(ION_Promise, "FAILED",    ION_PROMISOR_FAILED);
    PION_CLASS_CONST_LONG(ION_Promise, "FINISHED",  ION_PROMISOR_FINISHED);
    PION_CLASS_CONST_LONG(ION_Promise, "INTERNAL",  ION_PROMISOR_INTERNAL);
    PION_CLASS_CONST_LONG(ION_Promise, "TIMED_OUT", ION_PROMISOR_TIMED_OUT);
    PION_CLASS_CONST_LONG(ION_Promise, "CANCELED",  ION_PROMISOR_CANCELED);
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Promise) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Promise) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Promise) {
    return SUCCESS;
}