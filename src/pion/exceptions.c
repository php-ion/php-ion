#include "exceptions.h"
#include <spprintf.h>

zval * _pion_exception_new(zend_class_entry * exception_ce, const char * message, long code TSRMLS_DC) {
    zval *ex;
    MAKE_STD_ZVAL(ex);
    object_init_ex(ex, exception_ce);
    zend_class_entry *default_exception_ce = zend_exception_get_default(TSRMLS_C);
    if (message) {
        zend_update_property_string(default_exception_ce, ex, "message", sizeof("message")-1, message TSRMLS_CC);
    }
    if (code) {
        zend_update_property_long(default_exception_ce, ex, "code", sizeof("code")-1, code TSRMLS_CC);
    }
    return ex;
}

zval * _pion_exception_new_ex(zend_class_entry * exception_ce, long code TSRMLS_DC, const char * format, ...) {
    va_list arg;
    char *message;
    zval *zexception;

    va_start(arg, format);
    vspprintf(&message, 0, format, arg);
    va_end(arg);
    zexception = pion_exception_new(exception_ce, message, code);
    efree(message);
    return zexception;
}