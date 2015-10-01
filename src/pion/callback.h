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
int _pion_verify_arg_type(pionCb * cb, zend_uint arg_num, zval * arg TSRMLS_DC);


#define pion_cb_create(fci, fcc) pionCbCreate(fci, fcc TSRMLS_CC)
#define pion_cb_create_from_zval(zcb) pionCbCreateFromZval(zcb TSRMLS_CC)
#define pion_cb_free(cb) pionCbFree(cb)
#define pion_cb_num_args(cb) cb->fcc->function_handler->common.num_args
#define pion_cb_required_num_args(cb) cb->fcc->function_handler->common.required_num_args
#define pion_verify_arg_type(cb, arg_num, arg) _pion_verify_arg_type(cb, arg_num, arg TSRMLS_CC)

int _pion_fcall(zval ** result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval *** args TSRMLS_DC);
int _pion_call_void_fci_with_1_arg(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1 TSRMLS_DC);
int _pion_call_void_fci_with_2_args(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1, zval * arg2 TSRMLS_DC);
int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC, int num, ...);

#define pion_fcall_void_no_args(fci, fcc)   \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 0)
#define pion_fcall_void_1_arg(fci, fcc, arg)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 1, &arg)
#define pion_fcall_void_2_args(fci, fcc, arg1, arg2)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 2, &arg1, &arg2)
#define pion_fcall_void_3_args(fci, fcc, arg1, arg2, arg3)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 3, &arg1, &arg2, &arg3)

#define pion_call_fci(retval, fci, fcc, num, args)  \
    _pion_call_fci(retval, fci, fcc, num, args TSRMLS_CC)
#define pion_call_void_fci_with_1_arg(fci, fcc, arg1) \
    _pion_call_void_fci_with_1_arg(fci, fcc, arg1 TSRMLS_CC)
#define pion_call_void_fci_with_2_args(fci, fcc, arg1, arg2) \
    _pion_call_void_fci_with_2_args(fci, fcc, arg1, arg2 TSRMLS_CC)


zval * _pion_cb_call(pionCb *cb, int num, zval ***args TSRMLS_DC);
zval * _pion_cb_call_with_1_arg(pionCb * cb, zval* arg1 TSRMLS_DC);
zval * _pion_cb_call_with_2_args(pionCb *cb, zval *arg1, zval *arg2 TSRMLS_DC);
zval * _pion_cb_call_with_3_args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
zval * _pion_cb_call_with_4_args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC);

#define pion_cb_call(cb, num_args, args) \
    _pion_cb_call(cb, num_args, args TSRMLS_CC)
#define pion_cb_call_without_args(cb) \
    _pion_cb_call(cb, 0, NULL TSRMLS_CC)
#define pion_cb_call_with_1_arg(cb, arg) \
    _pion_cb_call_with_1_arg(cb, arg TSRMLS_CC)
#define pion_cb_call_with_2_args(cb, arg1, arg2) \
    _pion_cb_call_with_2_args(cb, arg1, arg2 TSRMLS_CC)
#define pion_cb_call_with_3_args(cb, arg1, arg2, arg3) \
    _pion_cb_call_with_3_args(cb, arg1, arg2, arg3 TSRMLS_CC)
#define pion_cb_call_with_4_args(cb, arg1, arg2, arg3, arg4) \
    _pion_cb_call_with_4_args(cb, arg1, arg2, arg3, arg4 TSRMLS_CC)


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

#define pion_new_object(ce, num_args, args)               pionNewObject(ce, num_args, args TSRMLS_CC)
#define pion_new_object_without_args(ce)                  pionNewObject(ce, 0 TSRMLS_CC)
#define pion_new_object_with_1_arg(ce, arg1)              pionNewObjectWith1Arg(ce, arg1 TSRMLS_CC)
#define pion_new_object_with_2_args(ce, arg1, arg2)       pionNewObjectWith2Args(ce, arg1, arg2 TSRMLS_CC)
#define pion_new_object_with_3_args(ce, arg1, arg2, arg3) pionNewObjectWith3Args(ce, arg1, arg2, arg3 TSRMLS_CC)

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
