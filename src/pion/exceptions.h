#ifndef PION_EXCEPTIONS_H
#define PION_EXCEPTIONS_H

#include <php.h>
#include <zend_exceptions.h>
#include <ext/spl/spl_exceptions.h>
#include "engine.h"

BEGIN_EXTERN_C()

//#define Exception zend_exception_get_default(TSRMLS_C);
#define cException zend_exception_get_default(TSRMLS_C)
#define getExceptionClass() zend_exception_get_default(TSRMLS_C)

#define _ion_get_class_Exception() zend_exception_get_default(TSRMLS_C)
#define ion_define_get_spl_class(cls) _ion_get_class_Exception()
zend_class_entry * ion_get_class(LogicException);

// Spl exceptions
#define _ion_get_class_LogicException()              spl_ce_LogicException
#define _ion_get_class_BadFunctionCallException()    spl_ce_BadFunctionCallException
#define _ion_get_class_BadMethodCallException()      spl_ce_BadMethodCallException
#define _ion_get_class_DomainException()             spl_ce_DomainException
#define _ion_get_class_InvalidArgumentException()    spl_ce_InvalidArgumentException
#define _ion_get_class_LengthException()             spl_ce_LengthException
#define _ion_get_class_OutOfRangeException()         spl_ce_OutOfRangeException
#define _ion_get_class_RuntimeException()            spl_ce_RuntimeException
#define _ion_get_class_OutOfBoundsException()        spl_ce_OutOfBoundsException
#define _ion_get_class_OverflowException()           spl_ce_OverflowException
#define _ion_get_class_RangeException()              spl_ce_RangeException
#define _ion_get_class_UnderflowException()          spl_ce_UnderflowException
#define _ion_get_class_UnexpectedValueException()    spl_ce_UnexpectedValueException
// basic ion exceptions
#define ion_get_class_ION_InvalidArgumentException() spl_ce_InvalidArgumentException
#define ion_get_class_ION_RuntimeException()         spl_ce_RuntimeException

zval * _pion_exception_new(zend_class_entry * exception_ce, const char * message, long code TSRMLS_DC);
zval * _pion_exception_new_ex(zend_class_entry * exception_ce, long code TSRMLS_DC, const char * message, ...);
/* do not export, in php it's available thru spprintf directly */
int zend_spprintf(char **message, int max_len, const char *format, ...);

#define pion_exception_new(ce, message, code)  _pion_exception_new(ce, message, code TSRMLS_CC)
#define pion_exception_new_ex(ce, code, format, ...)  _pion_exception_new_ex(ce, code TSRMLS_CC, format, ##__VA_ARGS__)
#define pion_throw(ce, message, code) zend_throw_exception(ce,  message, code TSRMLS_CC)
#define pion_throw_ex(ce, code, format, ...) zend_throw_exception_ex(class_name, code TSRMLS_CC, message, ##__VA_ARGS__)

#define Throw(class_name, message, code)                                            \
    zend_throw_exception(class_name,  message, code TSRMLS_CC);                     \

#define ThrowEx(class_name, code, message, ...)                                     \
    zend_throw_exception_ex(class_name, code TSRMLS_CC, message, ##__VA_ARGS__);    \

#define ThrowRuntime(message, code)     Throw(spl_ce_RuntimeException, message, code)
#define ThrowRuntimeEx(code, message, ...)     \
    ThrowEx(spl_ce_RuntimeException, code, message, ##__VA_ARGS__)
#define ThrowUnsupported(message)       ThrowRuntime(message, 1)
#define ThrowLogic(message, code)       Throw(spl_ce_LogicException, message, code)
#define ThrowLogicEx(code, message, ...)       \
    ThrowEx(spl_ce_LogicException, code, message, ##__VA_ARGS__)
#define ThrowInvalidArgument(message)   Throw(spl_ce_InvalidArgumentException, message, 1)
#define ThrowInvalidArgumentEx(message, ...)     \
    ThrowEx(spl_ce_InvalidArgumentException, 1, message, ##__VA_ARGS__)

END_EXTERN_C()

#endif //PION_EXCEPTIONS_H
