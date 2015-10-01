#include "Promise.h"
#include "Deferred.h"

ION_DEFINE_CLASS(ION_Promise);

void _ion_promise_resolve(zval * zpromise, zval * data, short type TSRMLS_CC) {
    ion_promise * promise = getInstance(zpromise);
    zval        * helper = NULL;
    zval        * retval = NULL;
    zval        * result = NULL;
    zend_bool     result_type = ION_DEFERRED_DONE;
    zend_bool     resume = 1;
    zend_class_entry * deferred_ce = ion_get_class(ION_Deferred);
    zend_class_entry * promise_ce = ion_get_class(ION_Promise);

    zval_add_ref(&zpromise);
    zval_add_ref(&data);
    promise->result = data;
    if(type == ION_DEFERRED_DONE) {
        if(promise->done) {
            if(pion_cb_num_args(promise->done)) { // has arguments
                if(pion_verify_arg_type(promise->done, 0, data) == SUCCESS) {
                    retval = pion_cb_call_with_1_arg(promise->done, data);
                }
            } else {
                retval = pion_cb_call_without_args(promise->done);
            }
        }
    } else {
        if(promise->fail) {
            if(pion_verify_arg_type(promise->fail, 0, data) == SUCCESS) {
                retval = pion_cb_call_with_1_arg(promise->fail, data);
            }
        } else if(promise->flags & ION_PROMISE_HAS_DONE_WITH_FAIL) {
            if(pion_verify_arg_type(promise->done, 1, data) == SUCCESS) {
                ALLOC_INIT_ZVAL(helper);
                retval = pion_cb_call_with_2_args(promise->done, helper, data);
                zval_ptr_dtor(&helper);
            }
        } else {
            result_type = ION_DEFERRED_FAIL;
        }
    }

    if(retval == NULL) {
        if(EG(exception)) {
            result = EG(exception);
            EG(exception) = NULL;
            result_type = ION_DEFERRED_FAIL;
        } else {
            // result = data;
            ALLOC_INIT_ZVAL(result);
            MAKE_COPY_ZVAL(&data, result);
        }
    } else {
        result = retval;
    }

    if(result) {
        if(Z_TYPE_P(result) == IS_OBJECT) {
            if(Z_OBJCE_P(result) == deferred_ce) { // is ION\Deferred
                zval_add_ref(&result);
                promise->await = result;
                resume = 0;
            } else if(Z_OBJCE_P(result) == promise_ce) { // is ION\Promise
                zval_add_ref(&result);
                promise->await = result;
                resume = 0;
            } else {

            }
        }

        if(promise->childs_count && resume) {
            for(ushort i = 0; i < promise->childs_count; i++) {
                ion_promise_resolve(promise->childs[i], result, result_type);
                zval_ptr_dtor(&promise->childs[i]);
                promise->childs[i] = NULL;
            }

            efree(promise->childs);
            promise->childs = NULL;
            promise->childs_count = 0;
        }
        zval_ptr_dtor(&result);
    }

    zval_ptr_dtor(&zpromise);
}

int _ion_promise_set_callback(zval * zpromise, zval * zdone, zval * zfail, zval * zprogress TSRMLS_DC) {
    ion_promise * promise = getInstance(zpromise);
    if(zdone) {
        promise->done = pion_cb_create_from_zval(zdone);
        promise->flags |= ION_PROMISE_HAS_DONE;

        if(pion_cb_num_args(promise->done) > 1) {
            if(pion_cb_required_num_args(promise->done) > 1) {

                return NULL;
            }
            promise->flags |= ION_PROMISE_HAS_DONE_WITH_FAIL;
        }
    }
    if(zfail) {
        promise->fail = pion_cb_create_from_zval(zfail);
        promise->flags |= ION_PROMISE_HAS_FAIL;
    }
    if(zprogress) {
        promise->progress = pion_cb_create_from_zval(zprogress);
        promise->flags |= ION_PROMISE_HAS_PROGRESS;
    }
    return SUCCESS;
}

zval * _ion_promise_push_callbacks(zval * zpromise, zval * zdone_cb, zval * zfail_cb, zval * zprogress_cb TSRMLS_DC) {
    ion_promise * promise = getInstance(zpromise);
    zval        * zchild = NULL;
    ALLOC_INIT_ZVAL(zchild);
    object_init_ex(zchild, CE(ION_Promise) TSRMLS_CC);
    ion_promise_set_callbacks(zchild, zdone_cb, zfail_cb, zprogress_cb);
    PION_PUSH_TO_ARRAY(promise->childs, promise->childs_count, zchild);
    return zchild;
}

CLASS_INSTANCE_CTOR(ION_Promise) {
    ion_promise *promise = emalloc(sizeof(ion_promise));
    memset(promise, 0, sizeof(ion_promise));
    RETURN_INSTANCE(ION_Promise, promise);
}

CLASS_INSTANCE_DTOR(ION_Promise) {
    ion_promise *promise = getInstanceObject(ion_promise *);
    if(promise->result) {
        zval_ptr_dtor(&promise->result);
    }
    efree(promise);
}

/** public function ION\Promise::__construct(callable $done = null, callable $fail = null, callable $progress = null) : int */
/** public function ION\Promise::__construct(callable $done = null, callable $progress = null) : int */
CLASS_METHOD(ION_Promise, __construct) {
//    ion_promise * promise = getThisInstance();
    zval * done = NULL;
    zval * fail = NULL;
    zval * progress = NULL;
    PARSE_ARGS("|z!z!z!", &done, &fail, &progress);
    ion_promise_set_callbacks(getThis(), done, fail, progress);
}

METHOD_ARGS_BEGIN(ION_Promise, __construct, 0)
    METHOD_ARG_CALLBACK(done, 0, 1)
    METHOD_ARG_CALLBACK(fail, 0, 1)
    METHOD_ARG_CALLBACK(progress, 0, 1)
METHOD_ARGS_END();

/** public function ION\Promise::then(callable $done = null, callable $fail = null, callable $progress = null) : ION\Promise */
CLASS_METHOD(ION_Promise, then) {
    zval        * zpromise = NULL;
    zval        * done = NULL;
    zval        * fail = NULL;
    zval        * progress = NULL;
    PARSE_ARGS("|z!z!z!", &done, &fail, &progress);
    zpromise = ion_promise_push_callbacks(getThis(), done, fail, progress);
    if(zpromise == NULL) {
        // throw
    }
    RETURN_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, then, 0)
    METHOD_ARG_CALLBACK(done, 0, 1)
    METHOD_ARG_CALLBACK(fail, 0, 1)
    METHOD_ARG_CALLBACK(progress, 0, 1)
METHOD_ARGS_END();

/** public function ION\Promise::onDone(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onDone) {
    zval * callback;
    zval * zpromise;
    PARSE_ARGS("z", &callback);
    zpromise = ion_promise_push_callbacks(getThis(), callback, NULL, NULL);
    if(zpromise == NULL) {
        // throw
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, onDone, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::onFail(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onFail) {
    zval * callback;
    zval * zpromise;
    PARSE_ARGS("z", &callback);
    zpromise = ion_promise_push_callbacks(getThis(), NULL, callback, NULL);
    if(zpromise == NULL) {
        // throw
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, onFail, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::onProgress(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, onProgress) {
    zval * callback;
    zval * zpromise;
    PARSE_ARGS("z", &callback);

    zpromise = ion_promise_push_callbacks(getThis(), NULL, NULL, callback);
    if(zpromise == NULL) {
        // throw
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, onProgress, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();


/** public function ION\Promise::__destruct() : int */
CLASS_METHOD(ION_Promise, __destruct) {
//    PHPDBG("Clean promise");
    ion_promise * promise = getThisInstance();
    if(promise->done) {
        pion_cb_free(promise->done);
        promise->done = NULL;
    }
    if(promise->fail) {
        pion_cb_free(promise->fail);
        promise->fail = NULL;
    }
    if(promise->progress) {
        pion_cb_free(promise->progress);
        promise->progress = NULL;
    }
    if(promise->childs_count) {
        for(uint i=0; i<promise->childs_count; i++) {
            zval_ptr_dtor(&promise->childs[i]);
        }
        efree(promise->childs);
        promise->childs_count = 0;
    }
    if(promise->result) {
        zval_ptr_dtor(&promise->result);
        promise->result = NULL;
    }
}

METHOD_WITHOUT_ARGS(ION_Promise, __destruct)


/** public function ION\Promise::done(mixed $data) : self */
CLASS_METHOD(ION_Promise, done) {
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

METHOD_ARGS_BEGIN(ION_Promise, done, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()

/** public function ION\Promise::fail(Exception $error) : self */
CLASS_METHOD(ION_Promise, fail) {
    ion_deferred *deferred = getThisInstance();
    zval *error;
    if(deferred->flags & ION_DEFERRED_FINISHED) {
        ThrowLogic("Failed to cancel finished defer-event", -1);
        return;
    }
    if(deferred->flags & ION_DEFERRED_INTERNAL) {
        ThrowLogic("Internal defer-event cannot be finished from user-space", -1);
        return;
    }
    PARSE_ARGS("z", &error);
    ion_promise_fail(getThis(), error);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Promise, fail, 1)
    METHOD_ARG_OBJECT(data, Exception, 0, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Promise)
    METHOD(ION_Promise, __construct,   ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, __destruct,    ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, then,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onDone,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onFail,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onProgress,    ZEND_ACC_PUBLIC)
#ifdef ION_DEBUG
    METHOD(ION_Promise, done,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, fail,          ZEND_ACC_PUBLIC)
#endif
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Promise) {
    PION_REGISTER_CLASS(ION_Promise, "ION\\Promise");
//    PION_REGISTER_EXTENDED_CLASS_WITH_CTOR(ION_Promise, ION_Deferred, "ION\\Promise");
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