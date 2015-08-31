#include "Debug.h"


DEFINE_CLASS(ION_Debug);

CLASS_METHOD(ION_Debug, fcallVoid) {

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