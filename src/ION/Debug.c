#include "Debug.h"

pionCb * global_cb;
DEFINE_CLASS(ION_Debug);

CLASS_METHOD(ION_Debug, fcallVoid) {

    zval * arg1 = NULL;
    zval * arg2 = NULL;
    zval * arg3 = NULL;
    int r = 0;
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("f|zzz", &fci, &fcc, &arg1, &arg2, &arg3);
    if(ZEND_NUM_ARGS() == 1) {
        r = pion_fcall_void_no_args(&fci, &fcc);
    } else if (ZEND_NUM_ARGS() == 2) {
        r = pion_fcall_void_1_arg(&fci, &fcc, arg1);
    } else if (ZEND_NUM_ARGS() == 3) {
        r = pion_fcall_void_2_args(&fci, &fcc, arg1, arg2);
    } else if (ZEND_NUM_ARGS() == 4) {
        r = pion_fcall_void_3_args(&fci, &fcc, arg1, arg2, arg3);
    }
    RETURN_LONG(r);
//    zval_ptr_dtor(&fci.function_name);
//    if (fci.object_ptr) {
//        zval_ptr_dtor(&fci.object_ptr);
//    }
}

METHOD_ARGS_BEGIN(ION_Debug, fcallVoid, 1)
    METHOD_ARG_TYPE(callback, IS_CALLABLE, 0, 0)
    METHOD_ARG(arg1, 0)
    METHOD_ARG(arg2, 0)
    METHOD_ARG(arg3, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Debug, cbCallVoid) {

    zval * arg1 = NULL;
    zval * arg2 = NULL;
    zval * arg3 = NULL;
    pionCb * cb = NULL;
    int r = 0;
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("f|zzz", &fci, &fcc, &arg1, &arg2, &arg3);
    cb = pionCbCreate(&fci, &fcc TSRMLS_CC);

    if(ZEND_NUM_ARGS() == 1) {
        r = pionCbVoidWithoutArgs(cb TSRMLS_CC);
    } else if (ZEND_NUM_ARGS() == 2) {
        r =pionCbVoidWith1Arg(cb, arg1 TSRMLS_CC);
    } else if (ZEND_NUM_ARGS() == 3) {
        r =pionCbVoidWith2Args(cb, arg1, arg2 TSRMLS_CC);
    } else if (ZEND_NUM_ARGS() == 4) {
        r =pionCbVoidWith3Args(cb, arg1, arg2, arg3 TSRMLS_CC);
    }

    pionCbFree(cb);
    RETURN_LONG(r);
}

METHOD_ARGS_BEGIN(ION_Debug, cbCallVoid, 1)
    METHOD_ARG_TYPE(callback, IS_CALLABLE, 0, 0)
    METHOD_ARG(arg1, 0)
    METHOD_ARG(arg2, 0)
    METHOD_ARG(arg3, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Debug, globalCbCall) {
    zval *zarg = NULL;
    PARSE_ARGS("z", &zarg);
    zval * result = pion_cb_call_with_1_arg(global_cb, zarg);
    pion_cb_free(global_cb);
    global_cb = NULL;
    RETURN_ZVAL(result, 0, 1);
}

METHOD_ARGS_BEGIN(ION_Debug, globalCbCall, 1)
    METHOD_ARG(arg, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Debug, globalCbObjCall) {
    zval * obj = NULL;
    zval * zarg = NULL;
    PARSE_ARGS("zz", &obj, &zarg);
    zval * result = pion_cb_obj_call_with_1_arg(global_cb, obj, zarg);
    pion_cb_free(global_cb);
    global_cb = NULL;
    RETURN_ZVAL(result, 0, 1);
}

METHOD_ARGS_BEGIN(ION_Debug, globalCbObjCall, 2)
    METHOD_ARG(obj, 0)
    METHOD_ARG(arg, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Debug, globalCbCallVoid) {
    zval *zarg = NULL;
    PARSE_ARGS("z", &zarg);
    int result = pionCbVoidWith1Arg(global_cb, zarg TSRMLS_CC);
    pionCbFree(global_cb);
    global_cb = NULL;
    RETURN_LONG((long)result);
}

METHOD_ARGS_BEGIN(ION_Debug, globalCbCallVoid, 1)
    METHOD_ARG(arg, 0)
METHOD_ARGS_END();


CLASS_METHOD(ION_Debug, globalCbCreate) {
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("f", &fci, &fcc);
    global_cb = pionCbCreate(&fci, &fcc TSRMLS_CC);
}

METHOD_ARGS_BEGIN(ION_Debug, globalCbCreate, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Debug, globalCbCreateFromZval) {
    zval *zcb = NULL;
    PARSE_ARGS("z", &zcb);
    global_cb = pion_cb_create_from_zval(zcb);
}

METHOD_ARGS_BEGIN(ION_Debug, globalCbCreateFromZval, 1)
    METHOD_ARG_CALLBACK(callback, 0, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Debug, globalCbFetchMethod) {
    char * class_name = NULL;
    char * class_name_len = 0;
    char * method_name = NULL;
    char * method_name_len = 0;
    PARSE_ARGS("ss", &class_name, &class_name_len, &method_name, &method_name_len);
    global_cb = pion_cb_fetch_method(class_name, method_name);
}

METHOD_ARGS_BEGIN(ION_Debug, globalCbFetchMethod, 2)
    METHOD_ARG_STRING(class_name, 0)
    METHOD_ARG_STRING(method_name, 0)
METHOD_ARGS_END();


CLASS_METHODS_START(ION_Debug)
    METHOD(ION_Debug, fcallVoid,              ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, cbCallVoid,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, globalCbCall,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, globalCbObjCall,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, globalCbCallVoid,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, globalCbCreate,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, globalCbCreateFromZval, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Debug, globalCbFetchMethod,    ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Debug) {
    PION_REGISTER_PLAIN_CLASS(ION_Debug, "ION\\Debug");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Debug) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Debug) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Debug) {
    return SUCCESS;
}