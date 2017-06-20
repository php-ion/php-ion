#include "ion.h"

zend_object_handlers ion_oh_ION_Deferred;
zend_class_entry * ion_ce_ION_Deferred;

zend_object * ion_deferred_zend_init(zend_class_entry * ce) {
    ion_promisor * deferred = ion_alloc_object(ce, ion_promisor);
    deferred->flags |= ION_PROMISOR_TYPE_DEFERRED;
    ZVAL_UNDEF(&deferred->object);
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
    ARGUMENT(cancel_callback, IS_CALLABLE)
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
    ion_promisor_cancel(deferred, message->val);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Deferred, cancel, 1)
    ARGUMENT(reason, IS_STRING)
METHOD_ARGS_END()

METHODS_START(methods_ION_Deferred)
    METHOD(ION_Deferred, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Deferred, cancel,      ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_Deferred) {
    ion_register_class_ex(&ion_ce_ION_Deferred, ion_ce_ION_ResolvablePromise, "ION\\Deferred", ion_deferred_zend_init, methods_ION_Deferred);
    ion_init_object_handlers(ion_oh_ION_Deferred);
    ion_oh_ION_Deferred.free_obj = ion_promisor_zend_free;
    ion_oh_ION_Deferred.clone_obj = ion_promisor_zend_clone;
    ion_oh_ION_Deferred.offset = ion_offset(ion_promisor);

//    ion_register_exception(ion_ce_ION_Deferred_RejectException, ion_ce_Exception, "ION\\Deferred\\RejectException", NULL);
//    ion_register_exception(ion_ce_ION_Deferred_TimeoutException, ion_ce_ION_Deferred_RejectException, "ION\\Deferred\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Deferred) {
    return SUCCESS;
}