#include "callback.h"
#include "debug.h"
#include "engine.h"
#include "exceptions.h"

/**
 * Create pionCb callback from FCI
 **/
pionCb *pionCbCreate(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC) {
    zval* retval_ptr;
    pionCb *cb = safe_emalloc(1, sizeof(pionCb), 0);
    memset(cb, 0,  sizeof(pionCb));
    cb->fci = safe_emalloc(1, sizeof(zend_fcall_info), 0);
    cb->fcc = safe_emalloc(1, sizeof(zend_fcall_info_cache), 0);

    memcpy(cb->fci, fci_ptr, sizeof(zend_fcall_info));
    memcpy(cb->fcc, fcc_ptr, sizeof(zend_fcall_info_cache));

    if (ZEND_FCI_INITIALIZED(*fci_ptr)) {
        Z_ADDREF_P(cb->fci->function_name);
        if (fci_ptr->object_ptr) {
            Z_ADDREF_P(fci_ptr->object_ptr);
        }
    }

    cb->fci->param_count = 0;
    cb->fci->no_separation = 0;
    cb->fci->retval_ptr_ptr = &retval_ptr;

    TSRMLS_SET_CTX(cb->thread_ctx);

    return cb;
}

pionCb *pionCbCreateFromZval(zval *zCb TSRMLS_DC) {
    zval * copy_cb;
//    ALLOC_INIT_ZVAL(copy_cb);
//    ZVAL_COPY_VALUE(zCb, copy_cb);
    pionCb *cb = emalloc(sizeof(pionCb));
    char *is_callable_error = NULL;
    memset(cb, 0, sizeof(pionCb));
    cb->fci = safe_emalloc(1, sizeof(empty_fcall_info), 0);
    cb->fcc = safe_emalloc(1, sizeof(empty_fcall_info_cache), 0);
    *cb->fci = empty_fcall_info;
    *cb->fcc = empty_fcall_info_cache;
    zval_addref_p(zCb);
    if (zend_fcall_info_init(zCb, IS_CALLABLE_STRICT, cb->fci, cb->fcc, NULL, &is_callable_error TSRMLS_CC) == SUCCESS) {
        if (is_callable_error) {
            return NULL;
        } else {
            if (cb->fci->object_ptr) {
                zval_add_ref(&cb->fci->object_ptr);
            }
            return cb;
        }
    } else {
        return NULL;
    }
}

/**
 * Destroy pionCb callback
 * */
void pionCbFree(pionCb *cb) {
    if(cb->fcc) {
        efree(cb->fcc);
    }
    if (cb->fci && ZEND_FCI_INITIALIZED(*cb->fci)) {
        zval_ptr_dtor(&cb->fci->function_name);
        if (cb->fci->object_ptr) {
            zval_ptr_dtor(&cb->fci->object_ptr);
        }
    }
    if(cb->zcb) {
        zval_ptr_dtor(&cb->zcb);
    }
    efree(cb->fci);
    efree(cb);
}


int _pion_verify_arg_type(pionCb * cb, zend_uint arg_num, zval * arg TSRMLS_DC) {

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
            ce = zend_fetch_class(cur_arg_info->class_name, cur_arg_info->class_name_len, (ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD) TSRMLS_CC);
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

int _pion_fcall(zval ** result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval *** args TSRMLS_DC) {
    if (ZEND_FCI_INITIALIZED(*fci_ptr)) {
        fci_ptr->retval_ptr_ptr = result;
        fci_ptr->params = args;
        fci_ptr->no_separation = 0;
        fci_ptr->param_count = (zend_uint)num;
        return zend_call_function(fci_ptr, fcc_ptr TSRMLS_CC);
    } else {
        return FAILURE;
    }
}

int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr TSRMLS_DC, int num, ...) {
    zval ** args[num];
    va_list args_list;
    zval * result = NULL;

    va_start(args_list, num);
    for (int j = 0; j < num; j++) {
        args[j] = va_arg(args_list, zval **);
    }
    va_end(args_list);
    int r =  _pion_fcall(&result, fci_ptr, fcc_ptr, num, args TSRMLS_CC);

    if(result) {
        zval_ptr_dtor(&result);
    }
    return r;
}


int pionCbVoidCall(pionCb *cb, int num, zval ***args TSRMLS_DC) {
    zval *pretval = NULL;
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval_ptr_ptr = &pretval;
        cb->fci->params = args;
        cb->fci->param_count = (zend_uint)num;
        if(zend_call_function(cb->fci, cb->fcc TSRMLS_CC) == FAILURE) {
            return FAILURE;
        }
        if(pretval) {
            zval_ptr_dtor(&pretval);
        }
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

int pionCbVoidWithoutArgs(pionCb * cb TSRMLS_DC) {
    return pionCbVoidCall(cb, 0, NULL TSRMLS_CC);
}

int pionCbVoidWith1Arg(pionCb * cb, zval* arg1 TSRMLS_DC) {
    zval **args[1];
    args[0] = &arg1;
    return pionCbVoidCall(cb, 1, args TSRMLS_CC);
}

int pionCbVoidWith2Args(pionCb *cb, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval **args[2];
    args[0] = &arg1;
    args[1] = &arg2;
    return pionCbVoidCall(cb, 2, args TSRMLS_CC);
}

int pionCbVoidWith3Args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval **args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    return pionCbVoidCall(cb, 3, args TSRMLS_CC);
}

int pionCbVoidWith4Args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC) {
    zval **args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    return pionCbVoidCall(cb, 4, args TSRMLS_CC);
}


zval * _pion_cb_call(pionCb *cb, int num, zval ***args TSRMLS_DC) {
    zval *pretval = NULL;
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval_ptr_ptr = &pretval;
        cb->fci->params = args;
        cb->fci->param_count = (zend_uint)num;
        if(zend_call_function(cb->fci, cb->fcc TSRMLS_CC) == FAILURE) {
            return NULL;
        }
        return pretval;
    } else {
        return NULL;
    }
}

zval * _pion_cb_call_with_1_arg(pionCb * cb, zval* arg1 TSRMLS_DC) {
    zval **args[1];
    args[0] = &arg1;
    return pion_cb_call(cb, 1, args);
}

zval * _pion_cb_call_with_2_args(pionCb *cb, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval **args[2];
    args[0] = &arg1;
    args[1] = &arg2;
    return pion_cb_call(cb, 2, args);
}

zval * _pion_cb_call_with_3_args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval **args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    return pion_cb_call(cb, 3, args);
}

zval * _pion_cb_call_with_4_args(pionCb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4 TSRMLS_DC) {
    zval **args[4];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    args[3] = &arg4;
    return pion_cb_call(cb, 4, args);
}

/**
 * Invoke class constructor
 * @param zend_class_entry* cls class entry
 * @param zval* this_ptr $this
 * @param int args_num
 * @param zval*** args
 * @return
 */
int pionCallConstructor(zend_class_entry *cls, zval *this_ptr, int args_num, zval ***args TSRMLS_DC) {
    zval *retval_ptr = NULL;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    if (!(cls->constructor->common.fn_flags & ZEND_ACC_PUBLIC)) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 1, "Invocation of %s's constructor failed",  cls->name);
        return FAILURE;
    }
    fci.size = sizeof(fci);
    fci.function_table = EG(function_table);
    fci.function_name = NULL;
    fci.symbol_table = NULL;
    fci.object_ptr = this_ptr;
    fci.retval_ptr_ptr = &retval_ptr;
    fci.param_count = (zend_uint)args_num;
    fci.params = args;
    fci.no_separation = 1;

    fcc.initialized = 1;
    fcc.function_handler = cls->constructor;
    fcc.calling_scope = EG(scope);
    fcc.called_scope = Z_OBJCE_P(this_ptr);
    fcc.object_ptr = this_ptr;

    if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
        if (retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        }
        if(!EG(exception)) {
            zend_throw_exception_ex(spl_ce_RuntimeException, 1, "Invocation of %s's constructor failed",  cls->name);
        }
        return FAILURE;
    }
    if (retval_ptr) {
        zval_ptr_dtor(&retval_ptr);
    }

    return SUCCESS;
}

int pionCallConstructorWith1Arg(zend_class_entry *cls, zval *this_ptr, zval *arg1 TSRMLS_DC) {
    zval **args[1];
    args[0] = &arg1;
    return pionCallConstructor(cls, this_ptr, 1, args TSRMLS_CC);
}

int pionCallConstructorWith2Args(zend_class_entry *cls, zval *this_ptr, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval **args[2];
    args[0] = &arg1;
    args[1] = &arg2;
    return pionCallConstructor(cls, this_ptr, 2, args TSRMLS_CC);
}

int pionCallConstructorWith3Args(zend_class_entry *cls, zval *this_ptr, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval **args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    return pionCallConstructor(cls, this_ptr, 3, args TSRMLS_CC);
}

zval* pionNewObject(zend_class_entry *ce, int args_num, zval ***args TSRMLS_DC) {
    zval *object = NULL;
    ALLOC_INIT_ZVAL(object);
    object_init_ex(object, ce TSRMLS_CC);
    if(ce->constructor) {
        if(pionCallConstructor(ce, object, args_num, args TSRMLS_CC) == FAILURE) {
            zval_ptr_dtor(&object);
            return NULL;
        }
    }

    return object;
}

zval* pionNewObjectWithoutArgs(zend_class_entry *ce TSRMLS_DC) {
    return pionNewObject(ce, 0, NULL TSRMLS_CC);
}

zval* pionNewObjectWith1Arg(zend_class_entry *ce, zval *arg1 TSRMLS_DC) {
    zval **args[1];
    args[0] = &arg1;
    return pionNewObject(ce, 1, args TSRMLS_CC);
}

zval* pionNewObjectWith2Args(zend_class_entry *ce, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval **args[2];
    args[0] = &arg1;
    args[1] = &arg2;
    return pionNewObject(ce, 2, args TSRMLS_CC);
}

zval* pionNewObjectWith3Args(zend_class_entry *ce, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval **args[3];
    args[0] = &arg1;
    args[1] = &arg2;
    args[2] = &arg3;
    return pionNewObject(ce, 3, args TSRMLS_CC);
}

zval* pionInitException(zend_class_entry *cls, char *message, int code TSRMLS_DC) {
    zval *msg, *c, *exception;
    ALLOC_STRING_ZVAL(msg, message, 1);
    ALLOC_LONG_ZVAL(c, code);
    exception = pionNewObjectWith2Args(cls, msg, c TSRMLS_CC);
    zval_ptr_dtor(&msg);
    zval_ptr_dtor(&c);
    return exception;
}

zval* pionCallFunction(const char *function_name, int num_args, zval **args TSRMLS_DC) {
    zval *zfunc, *retval = NULL;
    ALLOC_INIT_ZVAL(retval);
    ALLOC_INIT_ZVAL(zfunc);
    ZVAL_STRING(zfunc, function_name, 1);
    if(call_user_function(EG(function_table), NULL, zfunc, retval, (zend_uint)num_args, args TSRMLS_CC) == FAILURE) {
        zval_ptr_dtor(&zfunc);
        zval_ptr_dtor(&retval);
        return NULL;
    } else {
        zval_ptr_dtor(&zfunc);
        return retval;
    }
}

zval* pionCallFunctionWith1Arg(const char *function_name, zval *arg1 TSRMLS_DC) {
    zval *args[1];
    args[0] = arg1;
    return pionCallFunction(function_name, 1, args TSRMLS_CC);
}

zval* pionCallFunctionWith2Args(const char *function_name, zval *arg1, zval *arg2 TSRMLS_DC) {
    zval *args[2];
    args[0] = arg1;
    args[1] = arg2;
    return pionCallFunction(function_name, 2, args TSRMLS_CC);
}

zval* pionCallFunctionWith3Args(const char *function_name, zval *arg1, zval *arg2, zval *arg3 TSRMLS_DC) {
    zval *args[3];
    args[0] = arg1;
    args[1] = arg2;
    args[2] = arg3;
    return pionCallFunction(function_name, 3, args TSRMLS_CC);
}
