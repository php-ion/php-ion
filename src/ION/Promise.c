#include "Promise.h"

int _ion_promise_set_callback(zval * zpromise, zval * zdone, zval * zfail, zval * zprogress TSRMLS_DC) {
    ion_promise * promise = getInstance(zpromise);
    if(zdone) {
        promise->done = pion_cb_create_from_zval(zdone);
        promise->flags |= ION_PROMISE_HAS_DONE;
        if(promise->done->fci->param_count > 1) {
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
    if(promise->childs_count) {
        promise->childs = erealloc(promise->childs, sizeof(zval *) * ++promise->childs_count);
        promise->childs[promise->childs_count - 1] = zchild;
    } else {
        promise->childs = emalloc(sizeof(zval *));
        promise->childs[0] = zchild;
        promise->childs_count = 1;
    }
    return zpromise;
}

CLASS_INSTANCE_CTOR(ION_Promise) {
    ion_promise *promise = emalloc(sizeof(ion_promise));
    memset(promise, 0, sizeof(ion_promise));
    promise->next = pion_llist_init();
    RETURN_INSTANCE(ION_Promise, promise);
}

CLASS_INSTANCE_DTOR(ION_Promise) {
    ion_promise *promise = getInstanceObject(ion_promise *);
    if(promise->result) {
        zval_ptr_dtor(&promise->result);
    }
    pion_llist_free(promise->next);
    efree(promise);
}

/** public function ION\Promise::__construct(callable $done = null, callable $fail = null, callable $progress = null) : int */
CLASS_METHOD(ION_Promise, __construct) {
    ion_promise * promise = getThisInstance();
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

/** public function ION\Promise::done(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, done) {
    zval * callback;
    zval * zpromise;
    PARSE_ARGS("z", &callback);
    zpromise = ion_promise_push_callbacks(getThis(), callback, NULL, NULL);
    if(zpromise == NULL) {
        // throw
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, done, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::fail(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, fail) {
    zval * callback;
    zval * zpromise;
    PARSE_ARGS("z", &callback);
    zpromise = ion_promise_push_callbacks(getThis(), NULL, callback, NULL);
    if(zpromise == NULL) {
        // throw
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, fail, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

/** public function ION\Promise::progress(callable $callback) : ION\Promise */
CLASS_METHOD(ION_Promise, progress) {
    zval * callback;
    zval * zpromise;
    PARSE_ARGS("z", &callback);

    zpromise = ion_promise_push_callbacks(getThis(), NULL, NULL, callback);
    if(zpromise == NULL) {
        // throw
    }
    RETVAL_ZVAL(zpromise, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Promise, progress, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();


/** public function ION\Promise::__destruct() : int */
CLASS_METHOD(ION_Promise, __destruct) {
//    PHPDBG("ION\\Promise::__destruct()");
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
}

METHOD_WITHOUT_ARGS(ION_Promise, __destruct)


CLASS_METHODS_START(ION_Promise)
    METHOD(ION_Promise, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, __destruct,  ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, then,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, done,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, fail,        ZEND_ACC_PUBLIC)
    METHOD(ION_Promise, progress,    ZEND_ACC_PUBLIC)
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