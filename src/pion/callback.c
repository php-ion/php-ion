#include "callback.h"
//#include "debug.h"
#include "engine.h"
#include "exceptions.h"


//#define object_ptr object

/**
 * Create pion_cb callback from FCI
 **/
pion_cb * _pion_cb_create(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC) {
//    zval* retval_ptr;
    pion_cb *cb = emalloc(sizeof(pion_cb));
    cb->fci = emalloc(sizeof(zend_fcall_info));
    cb->fcc = emalloc(sizeof(zend_fcall_info_cache));

    memcpy(cb->fci, fci_ptr, sizeof(zend_fcall_info));
    memcpy(cb->fcc, fcc_ptr, sizeof(zend_fcall_info_cache));
    zval_addref_p(&cb->fci->function_name);
//    ZVAL_UNDEF(&cb->zcb);
//    if (ZEND_FCI_INITIALIZED(*fci_ptr)) {
//        Z_ADDREF(cb->fci->function_name);
//        if (fci_ptr->object) {
//            Z_ADDREF(fci_ptr->object);
//        }
//    }
//    ZVAL_UNDEF(&cb->fci->function_name);
//    Z_TYPE_INFO(cb->fci->function_name) = IS_OBJECT;
    cb->fci->param_count = 0;
    cb->fci->no_separation = 1;
    cb->fci->retval = NULL;

    TSRMLS_SET_CTX(cb->thread_ctx);

    return cb;
}

pion_cb * _pion_cb_create_from_zval(zval * zcb TSRMLS_DC) {
    pion_cb *cb = emalloc(sizeof(pion_cb));
    char *is_callable_error = NULL;
    memset(cb, 0, sizeof(pion_cb));
    cb->fci = emalloc(sizeof(zend_fcall_info));
    cb->fcc = emalloc(sizeof(zend_fcall_info_cache));

    *cb->fci = empty_fcall_info;
    *cb->fcc = empty_fcall_info_cache;
    cb->fci->param_count = 0;
    cb->fci->params = NULL;
    zval_addref_p(zcb);

    if (zend_fcall_info_init(zcb, IS_CALLABLE_CHECK_NO_ACCESS | IS_CALLABLE_CHECK_SILENT, cb->fci, cb->fcc, NULL, &is_callable_error) == SUCCESS) {
        if (is_callable_error) {
            efree(is_callable_error);
            return NULL;
        } else {
            return cb;
        }
    } else {
        if (is_callable_error) {
            efree(is_callable_error);
        }
        return NULL;
    }
}

pion_cb * _pion_cb_fetch_method(const char * class_name, const char * method_name TSRMLS_DC) {
    pion_cb * cb;
    zend_function * fptr;
    zend_class_entry *ce;
//    zend_class_entry **pce;
    char * function_name_lc;
//    zend_string class_name_zs;
    zend_string * class_name_zs = zend_string_init(class_name, strlen(class_name), 0);
//    class_name_zs.val = class_name;
//    ZVAL_STRINGL
//    class_name_zs.len = strlen(class_name);
//    zend_string * method_name_zs = zend_string_init(method_name, strlen(method_name), 0);
//    zend_str zend_string_init();
//    ALLOC_STRING_ZVAL()

    ce = zend_lookup_class(class_name_zs);
    zend_string_free(class_name_zs);

    if (ce == NULL) {
        return NULL;
    }
//    ce = *pce;

    function_name_lc = zend_str_tolower_dup(method_name, (int) strlen(method_name));

    fptr = zend_hash_str_find_ptr(&ce->function_table, function_name_lc, (int) strlen(method_name));
    if(fptr == NULL) {
        efree(function_name_lc);
        return NULL;
    }
    efree(function_name_lc);

//    ALLOC_INIT_ZVAL(callable);
//    array_init(callable);
//    add_next_index_string(callable, class_name, 1);
//    add_next_index_string(callable, method_name, 1);

    cb = emalloc(sizeof(pion_cb));
    cb->fci = emalloc(sizeof(empty_fcall_info));
    cb->fcc = emalloc(sizeof(empty_fcall_info_cache));
    *cb->fci = empty_fcall_info;
    *cb->fcc = empty_fcall_info_cache;
    cb->fci->size = sizeof(zend_fcall_info);
    cb->fci->function_table = NULL;
    ZVAL_UNDEF(&cb->fci->function_name);
//    cb->fci->function_name = callable;
    cb->fci->symbol_table  = NULL;
    cb->fci->object        = NULL;
    cb->fci->retval        = NULL;
    cb->fci->param_count   = 0;
    cb->fci->params        = NULL;
    cb->fci->no_separation = 1;

    cb->fcc->initialized      = 1;
    cb->fcc->function_handler = fptr;
    cb->fcc->calling_scope    = NULL;
    cb->fcc->called_scope     = ce;
    cb->fcc->object           = NULL;

    return cb;
}

void _pion_cb_free(pion_cb *cb) {
    zval_ptr_dtor(&cb->fci->function_name);
    efree(cb->fcc);
    efree(cb->fci);
//    if(cb->zcb) {
//    }
    efree(cb);
}


int _pion_verify_arg_type(pion_cb * cb, zend_uint arg_num, zval * arg TSRMLS_DC) {

    zend_arg_info *cur_arg_info;
    zend_class_entry *ce;

    if(!cb->fcc->function_handler->common.arg_info) {
        return FAILURE;
    }

    if(arg_num <= cb->fcc->function_handler->common.num_args) {
        cur_arg_info = &cb->fcc->function_handler->common.arg_info[arg_num];
    } else if(cb->fcc->function_handler->common.fn_flags & ZEND_ACC_VARIADIC) {
        cur_arg_info = &cb->fcc->function_handler->common.arg_info[cb->fcc->function_handler->common.num_args-1];
    } else {
        return FAILURE;
    }

    if(!cur_arg_info->type_hint) {
        return SUCCESS;
    }

//    return SUCCESS;

    if(cur_arg_info->class_name) {
        if (Z_TYPE_P(arg) == IS_OBJECT) {
            ce = zend_fetch_class(cur_arg_info->class_name, (ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD) TSRMLS_CC);
            if (!ce || !instanceof_function(Z_OBJCE_P(arg), ce TSRMLS_CC)) {
                return FAILURE;
            }
        } else if (Z_TYPE_P(arg) != IS_NULL || !cur_arg_info->allow_null) {
            return FAILURE;
        }
    } else if(cur_arg_info->type_hint) {
        switch(cur_arg_info->type_hint) {
            case IS_ARRAY:
                if (Z_TYPE_P(arg) != IS_ARRAY && (Z_TYPE_P(arg) != IS_NULL || !(cur_arg_info->allow_null))) {
                    return FAILURE;
                }
                break;
            case IS_CALLABLE:
                if (!zend_is_callable(arg, IS_CALLABLE_CHECK_SILENT, NULL TSRMLS_CC) && (Z_TYPE_P(arg) != IS_NULL || !(cur_arg_info->allow_null))) {
                    return FAILURE;
                }
                break;
            default:
                return FAILURE;
        }
    }
    return SUCCESS;
}

int _pion_fcall(zval * result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval * args TSRMLS_DC) {
    if (ZEND_FCI_INITIALIZED(*fci_ptr)) {
        fci_ptr->retval = result;
        fci_ptr->params = args;
        fci_ptr->no_separation = 1;
        fci_ptr->param_count = (zend_uint)num;
        return zend_call_function(fci_ptr, fcc_ptr TSRMLS_CC);
    } else {
        return FAILURE;
    }
}

int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC, int num, ...) {
    zval args[num];
    va_list args_list;
    zval result;
    zval * arg;

    va_start(args_list, num);
    for (int j = 0; j < num; j++) {
        arg = va_arg(args_list, zval *);
        args[j] = *arg;
    }
    va_end(args_list);
    int r =  _pion_fcall(&result, fci_ptr, fcc_ptr, num, args TSRMLS_CC);

//    if(result) {
        zval_ptr_dtor(&result);
//    }
    return r;
}


int _pion_cb_void(pion_cb *cb, int num, zval *args TSRMLS_DC) {
    zval retval;
//    zval *pretval = NULL;
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval = &retval;
        cb->fci->params = args;
        cb->fci->param_count = (uint32_t)num;
        if(zend_call_function(cb->fci, cb->fcc TSRMLS_CC) == FAILURE) {
            return FAILURE;
        }
        zval_ptr_dtor(&retval);
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

int _pion_cb_void_with_1_arg(pion_cb * cb, zval* arg1 TSRMLS_DC) {
    zval args[1];
    args[0] = *arg1;
    return _pion_cb_void(cb, 1, args TSRMLS_CC);
}

int _pion_cb_void_with_2_args(pion_cb *cb, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return _pion_cb_void(cb, 2, args TSRMLS_CC);
}

int _pion_cb_void_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return _pion_cb_void(cb, 3, args TSRMLS_CC);
}

int _pion_cb_void_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC) {
    zval args[4];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    args[3] = *arg4;
    return _pion_cb_void(cb, 4, args TSRMLS_CC);
}


zval _pion_cb_call(pion_cb *cb, int num, zval *args TSRMLS_DC) {
    zval retval;
    ZVAL_UNDEF(&retval);
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval = &retval;
        cb->fci->params = args;
        cb->fci->param_count = (zend_uint)num;
        zend_call_function(cb->fci, cb->fcc TSRMLS_CC);
        cb->fci->params = NULL;
        cb->fci->param_count = 0;
    }
    return retval;
}

zval _pion_cb_call_with_1_arg(pion_cb * cb, zval* arg1 TSRMLS_DC) {
    zval args[1];
    args[0] = *arg1;
    return pion_cb_call(cb, 1, args);
}

zval _pion_cb_call_with_2_args(pion_cb *cb, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_cb_call(cb, 2, args);
}

zval _pion_cb_call_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return pion_cb_call(cb, 3, args);
}

zval _pion_cb_call_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC) {
    zval args[4];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    args[3] = *arg4;
    return pion_cb_call(cb, 4, args);
}

zval _pion_cb_obj_call(pion_cb *cb, zval * obj, int num, zval *args TSRMLS_DC) {
    zval retval;
    ZVAL_UNDEF(&retval);
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval = &retval;
        cb->fci->params = args;
        cb->fci->param_count = (zend_uint)num;
        if(obj) {
            Z_ADDREF_P(obj);
            cb->fci->object = Z_OBJ_P(obj);
            cb->fcc->object = Z_OBJ_P(obj);
            cb->fcc->calling_scope = Z_OBJCE_P(obj);
        }
        zend_call_function(cb->fci, cb->fcc TSRMLS_CC);
        if(cb->fcc->object) {
            Z_DELREF_P(obj);
            cb->fcc->object = NULL;
            cb->fcc->calling_scope = NULL;
            cb->fci->object = NULL;

        }
    }
    return retval;
}

zval _pion_cb_obj_call_with_1_arg(pion_cb * cb, zval * obj, zval* arg1 TSRMLS_DC) {
    zval args[1];
    args[0] = *arg1;
    return pion_cb_obj_call(cb, obj, 1, args);
}

zval _pion_cb_obj_call_with_2_args(pion_cb *cb, zval * obj, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_cb_obj_call(cb, obj, 2, args);
}

zval _pion_cb_obj_call_with_3_args(pion_cb *cb, zval * obj, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return pion_cb_obj_call(cb, obj, 3, args);
}

zval _pion_cb_obj_call_with_4_args(pion_cb *cb, zval * obj, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC) {
    zval args[4];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    args[3] = *arg4;
    return pion_cb_obj_call(cb, obj, 4, args);
}

/**
 * Invoke class constructor
 * @param zend_class_entry* cls class entry
 * @param zval* this_ptr $this
 * @param int args_num
 * @param zval*** args
 * @return
 */
int _pion_call_constructor(zend_class_entry *cls, zval *this_ptr, int args_num, zval *args TSRMLS_DC) {
    zval retval_ptr;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    if (!(cls->constructor->common.fn_flags & ZEND_ACC_PUBLIC)) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 1, "Invocation of %s's constructor failed",  cls->name);
        return FAILURE;
    }
    fci.size = sizeof(fci);
    fci.function_table = EG(function_table);
    ZVAL_UNDEF(&fci.function_name);
//    fci.function_name = NULL;
    fci.symbol_table = NULL;
    fci.object = Z_OBJ_P(this_ptr);
    fci.retval = &retval_ptr;
    fci.param_count = (zend_uint)args_num;
    fci.params = args;
    fci.no_separation = 1;

    fcc.initialized = 1;
    fcc.function_handler = cls->constructor;
    fcc.calling_scope = EG(scope);
    fcc.called_scope = Z_OBJCE_P(this_ptr);
    fcc.object = Z_OBJ_P(this_ptr);

    if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
        if(!EG(exception)) {
            zend_throw_exception_ex(spl_ce_RuntimeException, 1, "Invocation of %s's constructor failed",  cls->name);
        }
        return FAILURE;
    }
    zval_ptr_dtor(&retval_ptr);

    return SUCCESS;
}

zend_object * _pion_new_object(zend_class_entry *ce, int args_num, zval *args TSRMLS_DC) {
    zval object;

    object_init_ex(&object, ce);
    if(ce->constructor) {
        if(pion_call_constructor(ce, &object, args_num, args TSRMLS_CC) == FAILURE) {
            zval_ptr_dtor(&object);
            return NULL;
        }
    }

    return Z_OBJ(object);
}


zend_object * _pion_new_object_arg_1(zend_class_entry *ce, zval *arg1 TSRMLS_DC) {
    zval args[1];
    args[0] = *arg1;
    return pion_new_object(ce, 1, args);
}

zend_object * _pion_new_object_arg_2(zend_class_entry *ce, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_new_object(ce, 2, args);
}

zend_object * _pion_new_object_arg_3(zend_class_entry *ce, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return pion_new_object(ce, 3, args);
}
