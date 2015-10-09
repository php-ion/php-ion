#include "Result.h"

ION_DEFINE_CLASS(ION_Promise_Result);
CLASS_INSTANCE_DTOR(ION_Promise_Result);
CLASS_INSTANCE_CTOR(ION_Promise_Result);

CLASS_INSTANCE_CTOR(ION_Promise_Result) {
    ion_promise_result * promise_result = emalloc(sizeof(ion_promise_result));
    memset(promise_result, 0, sizeof(ion_promise_result));
    RETURN_INSTANCE(ION_Promise_Result, promise_result);
}

CLASS_INSTANCE_DTOR(ION_Promise_Result) {
    ion_promise_result * promise_result = getInstanceObject(ion_promise_result *);
    if(promise_result->data) {
        zval_ptr_dtor(&promise_result->data);
    }
    efree(promise_result);
}

/** public function ION\Promise::__construct(mixed $data) */
CLASS_METHOD(ION_Promise_Result, __construct) {
    ion_promise_result * promise_result = getThisInstance();
    zval * data = NULL;
    PARSE_ARGS("z", &data);
    Z_ADDREF_P(data);
    promise_result->data = data;
}

METHOD_ARGS_BEGIN(ION_Promise_Result, __construct, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END();

/** public function ION\Promise::__destruct() */
CLASS_METHOD(ION_Promise_Result, __destruct) {
    ion_promise_result * promise_result = getThisInstance();
    if(promise_result->data) {
        zval_ptr_dtor(&promise_result->data);
        promise_result->data = NULL;
    }
}

METHOD_WITHOUT_ARGS(ION_Promise_Result, __destruct)

/** public function ION\Promise::getData() : mixed */
CLASS_METHOD(ION_Promise_Result, getData) {
    ion_promise_result * promise_result = getThisInstance();
    if(promise_result->data) {
        RETURN_ZVAL(promise_result->data, 1, 0);
    } else {
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Promise_Result, getData)


CLASS_METHODS_START(ION_Promise_Result)
    METHOD(ION_Promise_Result, __construct,   ZEND_ACC_PUBLIC)
    METHOD(ION_Promise_Result, __destruct,    ZEND_ACC_PUBLIC)
    METHOD(ION_Promise_Result, getData,       ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Promise_Result) {
    PION_REGISTER_CLASS(ION_Promise_Result, "ION\\Promise\\Result");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Promise_Result) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Promise_Result) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Promise_Result) {
    return SUCCESS;
}