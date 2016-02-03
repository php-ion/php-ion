#ifndef PION_EXCEPTIONS_H
#define PION_EXCEPTIONS_H

#include <php.h>
#include "zts.h"
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
#define ion_ce_LogicException             spl_ce_LogicException

// ION basic exceptions
extern ZEND_API zend_class_entry * ion_ce_ION_RuntimeException;
extern ZEND_API zend_class_entry * ion_ce_ION_InvalidUsageException;

// ION subsystems exception
extern ZEND_API zend_class_entry * ion_ce_ION_DNSException;

zend_object * pion_exception_new(zend_class_entry * exception_ce, const char * message, long code);
zend_object * pion_exception_new_ex(zend_class_entry * exception_ce, long code, const char * message, ...);

END_EXTERN_C()

#endif //PION_EXCEPTIONS_H
