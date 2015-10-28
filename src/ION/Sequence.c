#include "../pion.h"

zend_object_handlers ion_oh_ION_Sequence;
zend_class_entry * ion_ce_ION_Sequence;

zend_object * ion_sequence_init(zend_class_entry * ce) {
    ion_promisor * promise = emalloc(sizeof(ion_promisor));
    memset(promise, 0, sizeof(ion_promisor));
    promise->flags |= ION_PROMISOR_TYPE_PROMISE | ION_PROMISOR_TYPE_SEQUENCE | ION_PROMISOR_PROTOTYPE;
    RETURN_INSTANCE(ION_Sequence, promise);
}


/** public function ION\Sequence::__construct(callable $handler) : self */
CLASS_METHOD(ION_Sequence, __construct) {
    zval * handler = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(handler)
    ZEND_PARSE_PARAMETERS_END();
    ion_promisor_set_callbacks(Z_OBJ_P(getThis()), handler, NULL, NULL);
}

METHOD_ARGS_BEGIN(ION_Sequence, __construct, 1)
    METHOD_ARG_CALLBACK(handler, 0, 0)
METHOD_ARGS_END()


/** public function ION\Sequence::__invoke(mixed $data) : void */
CLASS_METHOD(ION_Sequence, __invoke) {
    zval * data = NULL;
    zend_object * clone = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(data)
    ZEND_PARSE_PARAMETERS_END();
    clone = ion_promisor_clone(Z_OBJ_P(getThis()));
    ion_promisor_done(clone, data);
}

METHOD_ARGS_BEGIN(ION_Sequence, __invoke, 1)
   METHOD_ARG(data, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Sequence)
    METHOD(ION_Sequence, __construct,       ZEND_ACC_PUBLIC)
    METHOD(ION_Sequence, __invoke,          ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Sequence) {
    pion_register_extended_class(ION_Sequence, ion_class_entry(ION_Promise), "ION\\Sequence", ion_sequence_init, CLASS_METHODS(ION_Sequence));
    pion_init_std_object_handlers(ION_Sequence);
    pion_set_object_handler(ION_Sequence, free_obj, ion_promisor_free);
    pion_set_object_handler(ION_Sequence, clone_obj, ion_promisor_clone_obj);
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Sequence) {
    return SUCCESS;
}
