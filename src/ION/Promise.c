#include "Promise.h"
#include "Deferred.h"
#include "Promise/Result.h"
#include <Zend/zend_generators.h>

ION_DEFINE_CLASS(ION_Promise);
CLASS_INSTANCE_DTOR(ION_Promise);
CLASS_INSTANCE_CTOR(ION_Promise);
// pre-cache methods
pionCb * generator_send    = NULL;
pionCb * generator_throw   = NULL;
pionCb * generator_current = NULL;
pionCb * generator_key     = NULL;
pionCb * generator_valid   = NULL;
pionCb * generator_result  = NULL;

void _ion_promise_resolve(zval * zpromise, zval * data, short type TSRMLS_DC) {
    ion_promise * promise = getInstance(zpromise);
    zval        * helper = NULL;
    zval        * retval = NULL;
    zval        * result = NULL;
    short         result_type = ION_DEFERRED_DONE;
    zend_bool     resolved = 1;
    zval        * is_valid = NULL;
    zval        * next = NULL;
    zend_bool     is_valid_generator = 0;
    zend_class_entry * object_ce         = NULL;
    zend_class_entry * deferred_ce       = ion_get_class(ION_Deferred);
    zend_class_entry * promise_ce        = ion_get_class(ION_Promise);
    zend_class_entry * generator_ce      = ion_get_class(Generator);
    zend_class_entry * deferred_map_ce   = NULL;
    zend_class_entry * promise_result_ce = ion_get_class(ION_Promise_Result);

    zval * _tmp1;
    zval * _tmp2;

    Z_ADDREF_P(zpromise);
    if(promise->await) {
        result_type = type;
        zval_ptr_dtor(&promise->await);
        promise->await = NULL;
        zval_ptr_dtor(&zpromise);
    } else {
        if (type == ION_DEFERRED_DONE) {
            if (promise->done) {
                if (pion_cb_num_args(promise->done)) { // has arguments
                    if (pion_verify_arg_type(promise->done, 0, data) == SUCCESS) {
                        retval = pion_cb_call_with_1_arg(promise->done, data);
                    }
                } else {
                    retval = pion_cb_call_without_args(promise->done);
                }
            }
        } else {
            if (promise->fail) {
                if (pion_verify_arg_type(promise->fail, 0, data) == SUCCESS) {
                    retval = pion_cb_call_with_1_arg(promise->fail, data);
                }
            } else if (promise->flags & ION_PROMISE_HAS_DONE_WITH_FAIL) {
                if (pion_verify_arg_type(promise->done, 1, data) == SUCCESS) {
                    ALLOC_INIT_ZVAL(helper);
                    retval = pion_cb_call_with_2_args(promise->done, helper, data);
                    zval_ptr_dtor(&helper);
                }
            } else {
                result_type = ION_DEFERRED_FAIL;
            }
        }
    }

    if(retval == NULL) {
        if(EG(exception)) {
            result = EG(exception);
            EG(exception) = NULL;
            result_type = ION_DEFERRED_FAIL;
        } else {
            ALLOC_INIT_ZVAL(result);
            MAKE_COPY_ZVAL(&data, result);
        }
    } else {
        result = retval;
    }
    if(result) {
        resolved = 1;
        watch_result:

        if(Z_TYPE_P(result) == IS_OBJECT) {
            if(Z_OBJCE_P(result) == deferred_ce) { // ION\Deferred
                ion_deferred * await = getInstance(result);
                if(await->flags & ION_DEFERRED_FINISHED) {
                    // todo: extract result and goto watch_result
                } else {
                    promise->await = result;
                    PION_PUSH_TO_ARRAY(await->handlers, await->handlers_count, zpromise);
                    zval_add_ref(&zpromise);
                    resolved = 0;
                }
            } else if(Z_OBJCE_P(result) == promise_ce) { // is ION\Promise
                ion_promise * await = getInstance(result);
                if(await->flags & ION_DEFERRED_FINISHED) {
                    // todo: extract result and goto watch_result
                } else {
                    promise->await = result;
                    PION_PUSH_TO_ARRAY(await->handlers, await->handler_count, zpromise);
                    zval_add_ref(&zpromise);
                    resolved = 0;
                }
            } else if(Z_OBJCE_P(result) == generator_ce) {
                if(promise->generator) { // push the generator to stack
                    PION_PUSH_TO_ARRAY(promise->generators_stack, promise->generators_count, promise->generator);
                }
                promise->generator = result;
                result = pion_cb_obj_call_without_args(generator_current, promise->generator);
                goto watch_result;
            } else if(Z_OBJCE_P(result) == deferred_map_ce) {
            } else if(Z_OBJCE_P(result) == promise_result_ce) {
                if(promise->generator_result) {
                    zval_ptr_dtor(&promise->generator_result);
                }
                ion_promise_result * promise_result = getInstance(result);
                ALLOC_INIT_ZVAL(promise->generator_result);
                MAKE_COPY_ZVAL(&promise_result->data, promise->generator_result);
                zval_ptr_dtor(&result);
                ALLOC_INIT_ZVAL(result);
                resolved = 1;
            }
        } else {
            resolved = 1;
        }
        if(promise->generator && resolved) {
            resume_generator: {
                if(result_type == ION_DEFERRED_DONE) {
                    next = pion_cb_obj_call_with_1_arg(generator_send, promise->generator, result);
                } else {
                    next = pion_cb_obj_call_with_1_arg(generator_throw, promise->generator, result);
                }
                zval_ptr_dtor(&result);
                if(EG(exception)) {
                    result = EG(exception);
                    EG(exception) = NULL;
                    result_type = ION_DEFERRED_FAIL;
                    if(promise->generator_result) {
                        zval_ptr_dtor(&promise->generator_result);
                        promise->generator_result = NULL;
                    }
                    is_valid_generator = 0;
                } else {
                    result = next;
                    is_valid = pion_cb_obj_call_without_args(generator_valid, promise->generator);
                    is_valid_generator = Z_BVAL_P(is_valid);
                    zval_ptr_dtor(&is_valid);
                    if(!is_valid_generator) {
                        zval_ptr_dtor(&result);
                        result_type = ION_DEFERRED_DONE;
#ifdef HAS_GENERATOR_RESULT
                        result = pion_cb_obj_call_without_args(generator_result, promise->generator)
                        // todo
#endif
                        if(promise->generator_result) {
                            result = promise->generator_result;
                            promise->generator_result = NULL;
                        } else {
                            ALLOC_INIT_ZVAL(result);
                        }
                    }
                }

                if(!is_valid_generator) {
                    zval_ptr_dtor(&promise->generator);
                    if(promise->generators_count) { // has more generators?
                        promise->generator = promise->generators_stack[promise->generators_count - 1];
                        if(promise->generators_count == 1) {
                            efree(promise->generators_stack);
                            promise->generators_stack = NULL;
                            promise->generators_count = 0;
                        } else {
                            promise->generators_stack = erealloc(promise->generators_stack, sizeof(zval *) * --promise->generators_count);
                        }
                        goto resume_generator;
                    } else {
                        promise->generator = NULL;
                    }
                }
                goto watch_result;
            };
        }

        if(resolved) {
            promise->result = result;
            if(promise->handler_count) {
                for(ushort i = 0; i < promise->handler_count; i++) {
                    ion_promise_resolve(promise->handlers[i], result, result_type);
                    zval_ptr_dtor(&promise->handlers[i]);
                    promise->handlers[i] = NULL;
                }

                efree(promise->handlers);
                promise->handlers = NULL;
                promise->handler_count = 0;
            }
        }

//        zval_ptr_dtor(&result);
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
                return FAILURE;
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
    PION_PUSH_TO_ARRAY(promise->handlers, promise->handler_count, zchild);
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

/** public static function ION\Promise::result(mixed $data) : self */
CLASS_METHOD(ION_Promise, result) {
    zval * data = NULL;
    zval * result = NULL;
    PARSE_ARGS("z", &data);

    result = pion_new_object_with_1_arg(ion_get_class(ION_Promise_Result), data);
    RETURN_ZVAL(result, 0, 1);
}

METHOD_ARGS_BEGIN(ION_Promise, result, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()

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
        ion_throw_invalid_argument_exception("Can't promise");
        return;
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
        ion_throw_invalid_argument_exception("Can't promise");
        return;
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
        ion_throw_invalid_argument_exception("Can't promise");
        return;
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
        ion_throw_invalid_argument_exception("Can't promise");
        return;
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, onProgress, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();


/** public function ION\Promise::__destruct() : int */
CLASS_METHOD(ION_Promise, __destruct) {
    ion_promise * promise = getThisInstance();
//    PHPDBG("Clean promise %d", (int)promise->uid);
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
    if(promise->handler_count) {
        for(uint i=0; i<promise->handler_count; i++) {
            zval_ptr_dtor(&promise->handlers[i]);
        }
        efree(promise->handlers);
        promise->handler_count = 0;
    }
    if(promise->result) {
        zval_ptr_dtor(&promise->result);
        promise->result = NULL;
    }
}

METHOD_WITHOUT_ARGS(ION_Promise, __destruct)

#ifdef ION_DEBUG
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

/** public function ION\Promise::setUID(int $uid) : self */
CLASS_METHOD(ION_Promise, setUID) {
    ion_promise * promise = getThisInstance();
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
    METHOD(ION_Promise, result,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Promise, __construct,   ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, __destruct,    ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, then,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onDone,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onFail,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, onProgress,    ZEND_ACC_PUBLIC)
#ifdef ION_DEBUG
    METHOD(ION_Promise, done,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, fail,          ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, setUID,        ZEND_ACC_PUBLIC)
#endif
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Promise) {
    PION_REGISTER_CLASS(ION_Promise, "ION\\Promise");
//    PION_REGISTER_EXTENDED_CLASS_WITH_CTOR(ION_Promise, ION_Deferred, "ION\\Promise");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Promise) {
    generator_send    = pion_cb_fetch_method("Generator", "send");
    generator_current = pion_cb_fetch_method("Generator", "current");
    generator_key     = pion_cb_fetch_method("Generator", "key");
    generator_throw   = pion_cb_fetch_method("Generator", "throw");
    generator_valid   = pion_cb_fetch_method("Generator", "valid");
#ifdef HAS_GENERATOR_RESULT
    generator_result   = pion_cb_fetch_method("Generator", "getResult");
#endif
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Promise) {
    pion_cb_free(generator_send);
    pion_cb_free(generator_current);
    pion_cb_free(generator_key);
    pion_cb_free(generator_throw);
    pion_cb_free(generator_valid);
#ifdef HAS_GENERATOR_RESULT
    pion_cb_free(generator_result);
#endif
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Promise) {
    return SUCCESS;
}