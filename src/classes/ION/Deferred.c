#include "ion.h"

zend_object_handlers ion_oh_ION_Deferred;
zend_class_entry * ion_ce_ION_Deferred;

zend_object * ion_deferred_init(zend_class_entry * ce) {
    ion_promisor * deferred = ion_alloc_object(ce, ion_promisor);
    deferred->flags |= ION_PROMISOR_TYPE_DEFERRED;
    return ion_init_object(ION_OBJECT_ZOBJ(deferred), ce, &ion_oh_ION_Deferred);
}

/** public function ION\Deferred::__construct(callable $canceler = null, bool $protected = true) : self */
CLASS_METHOD(ION_Deferred, __construct) {
    ion_promisor * deferred = ION_THIS_OBJECT(ion_promisor);
    zval         * canceler = NULL;
    zend_bool      protected = true;
    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_DEREF_EX(canceler, 1, 0)
        Z_PARAM_BOOL(protected)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(canceler) {
        ion_promisor_set_php_cb(&deferred->canceler, pion_cb_create_from_zval(canceler));
    }
    if(protected) {
        // @todo save run scope
    }
}

METHOD_ARGS_BEGIN(ION_Deferred, __construct, 0)
    METHOD_ARG_TYPE(cancel_callback, IS_CALLABLE, 0, 0)
    ARGUMENT(canceleler, _IS_BOOL)
METHOD_ARGS_END();


/** public function ION\Deferred::cancel(string $reason) : self */
CLASS_METHOD(ION_Deferred, cancel) {
    ion_promisor * deferred = ION_THIS_OBJECT(ion_promisor);
    zend_string  * message = NULL;
    if(deferred->flags & ION_PROMISOR_FINISHED) {
        zend_throw_exception(ion_ce_ION_InvalidUsageException, ERR_ION_PROMISE_ALREADY_FINISHED, 0);
        return;
    }

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(message)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
    ion_promisor_cancel(Z_OBJ_P(getThis()), message->val);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, cancel, 1)
    METHOD_ARG(reason, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Deferred)
    METHOD(ION_Deferred, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, cancel,      ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Deferred) {
    pion_register_extended_class(ION_Deferred, ion_ce_ION_ResolvablePromise, "ION\\Deferred", ion_deferred_init, CLASS_METHODS(ION_Deferred));
    pion_init_std_object_handlers(ION_Deferred);
    pion_set_object_handler(ION_Deferred, free_obj, ion_promisor_free);
    pion_set_object_handler(ION_Deferred, clone_obj, ion_promisor_clone_obj);
    ion_class_set_offset(ion_oh_ION_Deferred, ion_promisor);
//    PION_REGISTER_EXTENDED_CLASS(ION_Deferred, ION_ResolvablePromise, "ION\\Deferred");
//    CE(ION_Deferred)->ce_flags |= ZEND_ACC_FINAL_CLASS;
//    PION_CLASS_CONST_LONG(ION_Deferred, "RESOLVED", ION_DEFERRED_DONE);
//    PION_CLASS_CONST_LONG(ION_Deferred, "FAILED", ION_DEFERRED_FAILED);
//    PION_CLASS_CONST_LONG(ION_Deferred, "FINISHED", ION_DEFERRED_FINISHED);
//    PION_CLASS_CONST_LONG(ION_Deferred, "INTERNAL", ION_DEFERRED_INTERNAL);
//    PION_CLASS_CONST_LONG(ION_Deferred, "TIMED_OUT", ION_DEFERRED_TIMED_OUT);
//    PION_CLASS_CONST_LONG(ION_Deferred, "REJECTED", ION_DEFERRED_REJECTED);

//    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_RejectException, Exception, "ION\\Deferred\\RejectException", NULL);
//    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_TimeoutException, ION_Deferred_RejectException, "ION\\Deferred\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Deferred) {
    return SUCCESS;
}