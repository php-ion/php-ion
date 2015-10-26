#ifndef PION_EXCEPTIONS_H
#define PION_EXCEPTIONS_H

#include <php.h>
#include <zend_exceptions.h>
#include <ext/spl/spl_exceptions.h>
#include "engine.h"

BEGIN_EXTERN_C()

// Adopt native exceptions for ion_get_class() macro
// Zend exceptions
#define ion_ce_Throwable           zend_ce_throwable
#define ion_ce_Exception           zend_ce_exception
#define ion_ce_ErrorException      zend_ce_error_exception
#define ion_ce_Error               zend_ce_error
#define ion_ce_ParseError          zend_ce_parse_error;
#define ion_ce_TypeError           zend_ce_type_error;
#define ion_ce_ArithmeticError     zend_ce_arithmetic_error;
#define ion_ce_DivisionByZeroError zend_ce_division_by_zero_error;

// SPL exceptions
#define ion_ce_InvalidArgumentException   spl_ce_InvalidArgumentException
#define ion_ce_BadMethodCallException     spl_ce_BadMethodCallException
#define ion_ce_RuntimeException           spl_ce_RuntimeException

// ION basic exceptions
#define ion_ce_ION_RuntimeException           spl_ce_RuntimeException
#define ion_ce_ION_InvalidUsageException      spl_ce_RuntimeException

zend_object * pion_exception_new(zend_class_entry * exception_ce, const char * message, long code);
zend_object * pion_exception_new_ex(zend_class_entry * exception_ce, long code, const char * message, ...);

//#define pion_throw(ce, message, code) zend_throw_exception(ce,  message, code TSRMLS_CC)
//#define pion_throw_invalid_argument_exception(message) zend_throw_exception(ion_get_class(ION_InvalidArgumentException),  message, -1 TSRMLS_CC)
//#define pion_throw_ex(ce, code, format, ...) zend_throw_exception_ex(class_name, code TSRMLS_CC, message, ##__VA_ARGS__)

//#define ion_throw_invalid_argument_exception(message) pion_throw(ion_get_class(ION_InvalidArgumentException), message, -1)
//
//#define Throw(class_name, message, code)                                            \
//    zend_throw_exception(class_name,  message, code TSRMLS_CC);                     \
//
//#define ThrowEx(class_name, code, message, ...)                                     \
//    zend_throw_exception_ex(class_name, code TSRMLS_CC, message, ##__VA_ARGS__);    \
//
//#define ThrowRuntime(message, code)     Throw(spl_ce_RuntimeException, message, code)
//#define ThrowRuntimeEx(code, message, ...)     \
//    ThrowEx(spl_ce_RuntimeException, code, message, ##__VA_ARGS__)
//#define ThrowUnsupported(message)       ThrowRuntime(message, 1)
//#define ThrowLogic(message, code)       Throw(spl_ce_LogicException, message, code)
//#define ThrowLogicEx(code, message, ...)       \
//    ThrowEx(spl_ce_LogicException, code, message, ##__VA_ARGS__)
//#define ThrowInvalidArgument(message)   Throw(spl_ce_InvalidArgumentException, message, 1)
//#define ThrowInvalidArgumentEx(message, ...)     \
//    ThrowEx(spl_ce_InvalidArgumentException, 1, message, ##__VA_ARGS__)

END_EXTERN_C()

#endif //PION_EXCEPTIONS_H
