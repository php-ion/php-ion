#include "exceptions.h"

zend_class_entry * ion_ce_ION_RuntimeException;
zend_object_handlers ion_oh_ION_RuntimeException;
zend_class_entry * ion_ce_ION_InvalidUsageException;
zend_object_handlers ion_oh_ION_InvalidUsageException;

PHP_MINIT_FUNCTION(exceptions) {
    PION_REGISTER_VOID_EXTENDED_CLASS(ION_RuntimeException, ion_class_entry(RuntimeException), "ION\\RuntimeException");
    PION_REGISTER_VOID_EXTENDED_CLASS(ION_InvalidUsageException, ion_class_entry(LogicException), "ION\\InvalidUsageException");
    return SUCCESS;
}

zend_object * pion_exception_new(zend_class_entry * exception_ce, const char * message, long code) {
    zval ex;
//    A(ex);
    object_init_ex(&ex, exception_ce);
//    zend_objects_new();
    if (message) {
        zend_update_property_string(exception_ce, &ex, "message", sizeof("message")-1, message);
    }
    if (code) {
        zend_update_property_long(exception_ce, &ex, "code", sizeof("code")-1, code);
    }
    return Z_OBJ(ex);
}

zend_object * pion_exception_new_ex(zend_class_entry * exception_ce, long code, const char * format, ...) {
    va_list arg;
    char *message;
    zend_object * exception;

    va_start(arg, format);
    vspprintf(&message, 0, format, arg);
    va_end(arg);
    exception = pion_exception_new(exception_ce, message, code);
    efree(message);
    return exception;
}