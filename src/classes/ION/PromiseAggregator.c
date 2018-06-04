#include "ion.h"

zend_class_entry * ion_ce_ION_PromiseAggregator;
zend_object_handlers ion_oh_ION_PromiseAggregator;


/** public function ION\ResolvablePromise::done(mixed $data) : self */
CLASS_METHOD(ION_ResolvablePromise, done) {

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_ResolvablePromise, done, 1)
    ARGUMENT(data, IS_MIXED)
METHOD_ARGS_END()


METHODS_START(methods_ION_ResolvablePromise)
    METHOD(ION_ResolvablePromise, done, ZEND_ACC_PUBLIC)
//    METHOD(ION_ResolvablePromise, fail, ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_ResolvablePromise) {
    ion_register_class_ex(&ion_ce_ION_ResolvablePromise, ion_ce_ION_Promise, "ION\\PromiseAggregator", ion_promise_zend_init, methods_ION_ResolvablePromise);
//    ion_init_object_handlers(ion_oh_ION_ResolvablePromise);
//    ion_oh_ION_ResolvablePromise.free_obj = ion_promisor_zend_free;
//    ion_oh_ION_ResolvablePromise.clone_obj = ion_promisor_zend_clone;
//    ion_oh_ION_ResolvablePromise.offset = ion_offset(ion_promisor);
    return SUCCESS;
}