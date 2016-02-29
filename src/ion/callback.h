#ifndef PION_CALLBACK_H
#define PION_CALLBACK_H

#include "init.h"

/* PHP callback */
typedef struct pion_cb {
    zend_fcall_info * fci;
    zend_fcall_info_cache * fcc;
} pion_cb;

/* Create native callback */
pion_cb   * pion_cb_create(zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr);
pion_cb   * pion_cb_create_from_zval(zval * zcb);
pion_cb   * pion_cb_fetch_method(const char * class_name, const char * method_name);
void        pion_cb_release(pion_cb * cb);
void        pion_cb_free(pion_cb *cb);
zend_bool   pion_verify_arg_type(pion_cb * cb, uint32_t arg_num, zval * arg);
pion_cb   * pion_cb_dup(pion_cb * proto);

#define pion_cb_num_args(cb)          cb->fcc->function_handler->common.num_args
#define pion_cb_required_num_args(cb) cb->fcc->function_handler->common.required_num_args
#define pion_cb_is_generator(cb)      (cb->fcc->function_handler->common.fn_flags & ZEND_ACC_GENERATOR)
#define pion_cb_uses_strict_types(cb) (cb->fcc->function_handler->common.fn_flags & ZEND_ACC_STRICT_TYPES)

int _pion_fcall(zval * result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval * args);
int _pion_call_void_fci_with_1_arg(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1);
int _pion_call_void_fci_with_2_args(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, zval * arg1, zval * arg2);
int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, int num, ...);

#define pion_fcall_void_no_args(fci, fcc)   \
    _pion_fcall_void(fci, fcc, 0)
#define pion_fcall_void_1_arg(fci, fcc, arg)    \
    _pion_fcall_void(fci, fcc, 1, arg)
#define pion_fcall_void_2_args(fci, fcc, arg1, arg2)    \
    _pion_fcall_void(fci, fcc, 2, arg1, arg2)
#define pion_fcall_void_3_args(fci, fcc, arg1, arg2, arg3)    \
    _pion_fcall_void(fci, fcc, 3, arg1, arg2, arg3)

zval pion_cb_call(pion_cb *cb, int num, zval *args);
zval pion_cb_call_with_1_arg(pion_cb * cb, zval* arg1);
zval pion_cb_call_with_2_args(pion_cb *cb, zval *arg1, zval *arg2);
zval pion_cb_call_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3);
zval pion_cb_call_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4);

zval pion_cb_obj_call(pion_cb *cb, zend_object * obj, int num, zval *args);
zval pion_cb_obj_call_with_1_arg(pion_cb * cb, zend_object * obj, zval* arg1);
zval pion_cb_obj_call_with_2_args(pion_cb *cb, zend_object * obj, zval *arg1, zval *arg2);
zval pion_cb_obj_call_with_3_args(pion_cb *cb, zend_object * obj, zval *arg1, zval *arg2, zval *arg3);

#define pion_cb_call_without_args(cb) \
    pion_cb_call(cb, 0, NULL)

#define pion_cb_obj_call_without_args(cb, obj) \
    pion_cb_obj_call(cb, obj, 0, NULL)


/* Call callbacks */
int pion_cb_void(pion_cb *cb, int num, zval *args);
int pion_cb_void_with_1_arg(pion_cb * cb, zval* arg1);
int pion_cb_void_with_2_args(pion_cb *cb, zval *arg1, zval *arg2);
int pion_cb_void_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3);
int pion_cb_void_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4);

#define pion_cb_void_without_args(cb)                        pion_cb_void(cb, 0, NULL)

int     pion_call_constructor(zend_class_entry * ce, zend_object * object, int num_args, zval * args);
#define pion_call_constructor_without_args(cls, object)  pion_call_constructor(cls, object, 0, NULL)

/* Create an object */

zend_object * pion_new_object(zend_class_entry * ce, int args_num, zval *args);
#define pion_new_object_arg_0(ce)                         pion_new_object(ce, 0, NULL)
#define pion_new_object_arg_1(ce, arg1)                   pion_new_object(ce, 1, arg1)
zend_object * pion_new_object_arg_2(zend_class_entry * ce, zval * arg1, zval * arg2);
#define pion_new_object_arg_3(ce, arg1, arg2, arg3)       _pion_new_object_arg_3(ce, arg1, arg2, arg3)


#endif //PION_CALLBACK_H
