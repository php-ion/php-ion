

#include "engine.h"
#include "Zend/zend_object_handlers.h"

//zval * _pion_clone_object(zval * obj TSRMLS_DC) {
//    zval * clone;
//    zend_class_entry *ce;
//    zend_function * clone_function;
//    zend_object_clone_obj_t clone_call;
//
//    ce = Z_OBJCE_P(obj);
//    clone_function = ce ? ce->clone : NULL;
//    clone_call =  Z_OBJ_HT_P(obj)->clone_obj;
//
//    if (UNEXPECTED(clone_call == NULL)) {
//        if (ce) {
//            zend_error_noreturn(E_ERROR, "Trying to clone an uncloneable object of class %s", ce->name);
//        } else {
//            zend_error_noreturn(E_ERROR, "Trying to clone an uncloneable object");
//        }
//    }
//
//    if (ce && clone_function) {
//        if (clone_function->op_array.fn_flags & ZEND_ACC_PRIVATE) {
//            /* Ensure that if we're calling a private function, we're allowed to do so.
//             */
//            if (UNEXPECTED(ce != EG(scope))) {
//                zend_error_noreturn(E_ERROR, "Call to private %s::__clone() from context '%s'", ce->name, EG(scope) ? EG(scope)->name : "");
//            }
//        } else if ((clone_function->common.fn_flags & ZEND_ACC_PROTECTED)) {
//            /* Ensure that if we're calling a protected function, we're allowed to do so.
//             */
//            if (UNEXPECTED(!zend_check_protected(
//                    clone_function->common.prototype ? clone_function->common.prototype->common.scope : clone_function->common.scope,
//                    EG(scope)))
//                    ) {
//                zend_error_noreturn(E_ERROR, "Call to protected %s::__clone() from context '%s'", ce->name, EG(scope) ? EG(scope)->name : "");
//            }
//        }
//    }
//
//    ALLOC_ZVAL(clone);
//    Z_OBJVAL_P(clone) = clone_call(obj TSRMLS_CC);
//    Z_TYPE_P(clone) = IS_OBJECT;
//    Z_SET_REFCOUNT_P(clone, 1);
//    Z_SET_ISREF_P(clone);
//
//    return clone;
//}