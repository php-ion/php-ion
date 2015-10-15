#ifndef PION_CALLBACK_H
#define PION_CALLBACK_H

#include <php.h>
#include "engine.h"
#ifdef ZTS
#  include "TSRM.h"
#endif

/* PHP callback */
typedef struct _pionCb {
    zend_fcall_info * fci;
    zend_fcall_info_cache * fcc;
    zval * zcb;
#ifdef ZTS
    void ***thread_ctx;
#endif
} pion_cb;

/* Create native callback */
pion_cb * _pion_cb_create(zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr TSRMLS_DC);
pion_cb * _pion_cb_create_from_zval(zval * zcb TSRMLS_DC);
pion_cb * _pion_cb_fetch_method(const char * class_name, const char * method_name TSRMLS_DC);
void      _pion_cb_free(pion_cb *cb TSRMLS_DC);
int       _pion_verify_arg_type(pion_cb * cb, zend_uint arg_num, zval * arg TSRMLS_DC);


#define pion_cb_create(fci, fcc) _pion_cb_create(fci, fcc TSRMLS_CC)
#define pion_cb_create_from_zval(zcb) _pion_cb_create_from_zval(zcb TSRMLS_CC)
#define pion_cb_fetch_method(class, method) _pion_cb_fetch_method(class, method TSRMLS_CC)
#define pion_cb_free(cb) _pion_cb_free(cb TSRMLS_CC)
#define pion_cb_num_args(cb) cb->fcc->function_handler->common.num_args
#define pion_cb_required_num_args(cb) cb->fcc->function_handler->common.required_num_args
#define pion_verify_arg_type(cb, arg_num, arg) _pion_verify_arg_type(cb, arg_num, arg TSRMLS_CC)

int _pion_fcall(zval * result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval * args TSRMLS_DC);
int _pion_call_void_fci_with_1_arg(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1 TSRMLS_DC);
int _pion_call_void_fci_with_2_args(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1, zval * arg2 TSRMLS_DC);
int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC, int num, ...);

#define pion_fcall_void_no_args(fci, fcc)   \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 0)
#define pion_fcall_void_1_arg(fci, fcc, arg)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 1, arg)
#define pion_fcall_void_2_args(fci, fcc, arg1, arg2)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 2, arg1, arg2)
#define pion_fcall_void_3_args(fci, fcc, arg1, arg2, arg3)    \
    _pion_fcall_void(fci, fcc TSRMLS_CC, 3, arg1, arg2, arg3)

#define pion_call_fci(retval, fci, fcc, num, args)  \
    _pion_call_fci(retval, fci, fcc, num, args TSRMLS_CC)
#define pion_call_void_fci_with_1_arg(fci, fcc, arg1) \
    _pion_call_void_fci_with_1_arg(fci, fcc, arg1 TSRMLS_CC)
#define pion_call_void_fci_with_2_args(fci, fcc, arg1, arg2) \
    _pion_call_void_fci_with_2_args(fci, fcc, arg1, arg2 TSRMLS_CC)
#define pion_call_void_fci_with_3_args(fci, fcc, arg1, arg2, arg3) \
    _pion_call_void_fci_with_3_args(fci, fcc, arg1, arg2, arg3 TSRMLS_CC)
#define pion_call_void_fci_with_4_args(fci, fcc, arg1, arg2, arg3, arg4) \
    _pion_call_void_fci_with_4_args(fci, fcc, arg1, arg2, arg3, arg4 TSRMLS_CC)


zval _pion_cb_call(pion_cb *cb, int num, zval *args TSRMLS_DC);
zval _pion_cb_call_with_1_arg(pion_cb * cb, zval* arg1 TSRMLS_DC);
zval _pion_cb_call_with_2_args(pion_cb *cb, zval *arg1, zval *arg2 TSRMLS_DC);
zval _pion_cb_call_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
zval _pion_cb_call_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC);

zval _pion_cb_obj_call(pion_cb *cb, zval * obj, int num, zval *args TSRMLS_DC);
zval _pion_cb_obj_call_with_1_arg(pion_cb * cb, zval * obj, zval* arg1 TSRMLS_DC);
zval _pion_cb_obj_call_with_2_args(pion_cb *cb, zval * obj, zval *arg1, zval *arg2 TSRMLS_DC);
zval _pion_cb_obj_call_with_3_args(pion_cb *cb, zval * obj, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
zval _pion_cb_obj_call_with_4_args(pion_cb *cb, zval * obj, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC);

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

#define pion_cb_obj_call(cb, obj, num_args, args) \
    _pion_cb_obj_call(cb, obj, num_args, args TSRMLS_CC)
#define pion_cb_obj_call_without_args(cb, obj) \
    _pion_cb_obj_call(cb, obj, 0, NULL TSRMLS_CC)
#define pion_cb_obj_call_with_1_arg(cb, obj, arg) \
    _pion_cb_obj_call_with_1_arg(cb, obj, arg TSRMLS_CC)
#define pion_cb_obj_call_with_2_args(cb, obj, arg1, arg2) \
    _pion_cb_obj_call_with_2_args(cb, obj, arg1, arg2 TSRMLS_CC)
#define pion_cb_obj_call_with_3_args(cb, obj, arg1, arg2, arg3) \
    _pion_cb_obj_call_with_3_args(cb, obj, arg1, arg2, arg3 TSRMLS_CC)
#define pion_cb_obj_call_with_4_args(cb, obj, arg1, arg2, arg3, arg4) \
    _pion_cb_obj_call_with_4_args(cb, obj, arg1, arg2, arg3, arg4 TSRMLS_CC)


/* Call callbacks */
int _pion_cb_void(pion_cb *cb, int num, zval *args TSRMLS_DC);
int _pion_cb_void_without_args(pion_cb * cb TSRMLS_DC);
int _pion_cb_void_with_1_arg(pion_cb * cb, zval* arg1 TSRMLS_DC);
int _pion_cb_void_with_2_args(pion_cb *cb, zval *arg1, zval *arg2 TSRMLS_DC);
int _pion_cb_void_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);
int _pion_cb_void_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC);

#define pion_cb_void(cb, num_args, args)                     _pion_cb_void(cb, num_args, args TSRMLS_CC)
#define pion_cb_void_without_args(cb)                        _pion_cb_void(cb, 0, NULL TSRMLS_CC)
#define pion_cb_void_with_1_arg(cb, arg1)                    _pion_cb_void_with_1_arg(cb, arg1 TSRMLS_CC)
#define pion_cb_void_with_2_args(cb, arg1, arg2)             _pion_cb_void_with_2_args(cb, arg1, arg2 TSRMLS_CC)
#define pion_cb_void_with_3_args(cb, arg1, arg2, arg3)       _pion_cb_void_with_3_args(cb, arg1, arg2, arg3 TSRMLS_CC)
#define pion_cb_void_with_4_args(cb, arg1, arg2, arg3, arg4) _pion_cb_void_with_4_args(cb, arg1, arg2, arg3, arg4 TSRMLS_CC)

int   _pion_call_constructor(zend_class_entry *class_name, zval *object, int num_args, zval *args TSRMLS_DC);
#define pion_call_constructor(cls, object, num_args, args)  _pion_call_constructor(cls, object, num_args, args TSRMLS_CC)
#define pion_call_constructor_without_args(cls, object)  _pion_call_constructor(cls, object, 0, NULL TSRMLS_CC)
#define pion_call_constructor_with_1_arg(cls, object, arg1)  _pion_call_constructor(cls, object, 1, arg1 TSRMLS_CC)

/* Create an object */

#define pion_new_object(ce, num_args, args)               _pion_new_object(ce, num_args, args TSRMLS_CC)
#define pion_new_object_arg_0(ce)                         _pion_new_object(ce, 0, NULL TSRMLS_CC)
#define pion_new_object_arg_1(ce, arg1)                   _pion_new_object_arg_1(ce, arg1 TSRMLS_CC)
#define pion_new_object_arg_2(ce, arg1, arg2)             _pion_new_object_arg_2(ce, arg1, arg2 TSRMLS_CC)
#define pion_new_object_arg_3(ce, arg1, arg2, arg3)       _pion_new_object_arg_3(ce, arg1, arg2, arg3 TSRMLS_CC)

zend_object * _pion_new_object(zend_class_entry *ce, int num_args, zval *args TSRMLS_DC);
zend_object * _pion_new_object_arg_1(zend_class_entry *ce, zval *arg1 TSRMLS_DC);
zend_object * _pion_new_object_arg_2(zend_class_entry *ce, zval *arg1, zval *arg2 TSRMLS_DC);
zend_object * _pion_new_object_arg_3(zend_class_entry *ce, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);

//zval* pionInitException(zend_class_entry *cls, char *message, int code TSRMLS_DC);

/* Call PHP named function */
zval* pionCallFunction(const char *function_name, int num_args, zval **args TSRMLS_DC);
zval* pionCallFunctionWithoutArgs(const char *function_name, zval *arg1 TSRMLS_DC);
zval* pionCallFunctionWith1Arg(const char *function_name, zval *arg1 TSRMLS_DC);
zval* pionCallFunctionWith2Args(const char *function_name, zval *arg1, zval *arg2 TSRMLS_DC);
zval* pionCallFunctionWith3Args(const char *function_name, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC);

#endif //PION_CALLBACK_H
