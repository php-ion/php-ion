#ifndef PION_EXCEPTIONS_H
#define PION_EXCEPTIONS_H

#include <zend_exceptions.h>
#include <ext/spl/spl_exceptions.h>

BEGIN_EXTERN_C();

#define cException zend_exception_get_default(TSRMLS_C)

#define Throw(class_name, message, code)                                            \
    zend_throw_exception(class_name,  message, code TSRMLS_CC);                               \

#define ThrowEx(class_name, code, message, ...)                                     \
    zend_throw_exception_ex(class_name, code TSRMLS_CC, message, ##__VA_ARGS__);              \

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

END_EXTERN_C();

#endif //PION_EXCEPTIONS_H
