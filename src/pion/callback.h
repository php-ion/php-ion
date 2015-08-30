#ifndef PION_CALLBACK_H
#define PION_CALLBACK_H

#include <php.h>
#ifdef ZTS
#  include "TSRM.h"
#endif

/* PHP callback */
typedef struct _pionCb {
    zend_fcall_info *fci;
    zend_fcall_info_cache *fcc;
    zval *zcb;
#ifdef ZTS
    void ***thread_ctx;
#endif
} pionCb;

/* Create native callback */
pionCb * pionCbCreate(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC);
pionCb * pionCbCreateFromZval(zval *zCb TSRMLS_DC);
void   pionCbFree(pionCb *cb);

int _pion_fcall(zval * result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval *** args TSRMLS_DC);
int _pion_call_void_fci_with_1_arg(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1 TSRMLS_DC);
int _pion_call_void_fci_with_2_args(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1, zval * arg2 TSRMLS_DC);
int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC, int num, ...);

#define pion_fcall_void_no_args(fci, fcc)   \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 0)
#define pion_fcall_void_1_arg(fci, fcc, arg)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 1, &arg)
#define pion_fcall_void_2_args(fci, fcc, arg1, arg2)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 2, &arg1, &arg2)

#define pion_call_fci(retval, fci, fcc, num, args)  \
    _pion_call_fci(retval, fci, fcc, num, args TSRMLS_CC)
#define pion_call_void_fci_with_1_arg(fci, fcc, arg1) \
    _pion_call_void_fci_with_1_arg(fci, fcc, arg1 TSRMLS_CC)
#define pion_call_void_fci_with_2_args(fci, fcc, arg1, arg2) \
    _pion_call_void_fci_with_2_args(fci, fcc, arg1, arg2 TSRMLS_CC)


/* Call callbacks */
int pionCbVoidCall(pionCb *cb, int num, zval ***args TSRMLS_DC);
int pionCbVoidWithoutArgs(pionCb * cb TSRMLS_DC);
int pionCbVoidWith1Arg(pionCb * cb, zval* arg1 TSRMLS_DC);
int pionCbVoidWith2Args(pionCb *cb, zval *arg1, zval *arg2 TSRMLS_DC);
int pionCbVoidWith3Args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
int pionCbVoidWith4Args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC);

int   pionCallConstructor(zend_class_entry *class_name, zval *object, int num_args, zval ***args TSRMLS_DC);
#define pionCallConstructorWithoutArgs(cls, object)  pionCallConstructor(cls, object, 0, NULL TSRMLS_CC)

/* Create an object */
zval* pionNewObject(zend_class_entry *ce, int num_args, zval ***args TSRMLS_DC);
zval* pionNewObjectWithoutArgs(zend_class_entry *ce TSRMLS_DC);
zval* pionNewObjectWith1Arg(zend_class_entry *ce, zval *arg1 TSRMLS_DC);
zval* pionNewObjectWith2Args(zend_class_entry *ce, zval *arg1, zval *arg2 TSRMLS_DC);
zval* pionNewObjectWith3Args(zend_class_entry *ce, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);

zval* pionInitException(zend_class_entry *cls, char *message, int code TSRMLS_DC);

/* Call PHP named function */
zval* pionCallFunction(const char *function_name, int num_args, zval **args TSRMLS_DC);
zval* pionCallFunctionWithoutArgs(const char *function_name, zval *arg1 TSRMLS_DC);
zval* pionCallFunctionWith1Arg(const char *function_name, zval *arg1 TSRMLS_DC);
zval* pionCallFunctionWith2Args(const char *function_name, zval *arg1, zval *arg2 TSRMLS_DC);
zval* pionCallFunctionWith3Args(const char *function_name, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);

#endif //PION_CALLBACK_H
