#include "Debug.h"


DEFINE_CLASS(ION_Debug);

CLASS_METHOD(ION_Debug, fcallVoid) {

    zval * arg1 = NULL;
    zval * arg2 = NULL;
    zval * arg3 = NULL;
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("f|zzz", &fci, &fcc, &arg1, &arg2, &arg3);
    if(ZEND_NUM_ARGS() == 1) {
        pion_fcall_void_no_args(&fci, &fcc);
    } else if (ZEND_NUM_ARGS() == 2) {
        pion_fcall_void_1_arg(&fci, &fcc, arg1);
    } else if (ZEND_NUM_ARGS() == 3) {
        pion_fcall_void_2_args(&fci, &fcc, arg1, arg2);
    } else if (ZEND_NUM_ARGS() == 4) {
        pion_fcall_void_3_args(&fci, &fcc, arg1, arg2, arg3);
    }
}

METHOD_ARGS_BEGIN(ION_Debug, fcallVoid, 1)
    METHOD_ARG_TYPE(callback, IS_CALLABLE, 0, 0)
METHOD_ARGS_END();

CLASS_METHODS_START(ION_Debug)
    METHOD(ION_Debug, fcallVoid,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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