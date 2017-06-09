#include "ion.h"

zend_object_handlers ion_oh_ION_Sequence;
zend_class_entry * ion_ce_ION_Sequence;


zend_object * ion_sequence_zend_init(zend_class_entry * ce) {

    ion_promisor * promise = ion_alloc_object(ce, ion_promisor);
    promise->flags |= ION_PROMISOR_TYPE_PROMISE | ION_PROMISOR_TYPE_SEQUENCE | ION_PROMISOR_PROTOTYPE;
    ZVAL_UNDEF(&promise->object);
    return ion_init_object(ION_OBJECT_ZOBJ(promise), ce, &ion_oh_ION_Sequence);
}

/** public function ION\Sequence::__construct(callable $starter = null, callable $release = null) */
CLASS_METHOD(ION_Sequence, __construct) {
    ion_promisor * sequence = ION_THIS_OBJECT(ion_promisor);
    zval         * starter = NULL;
    zval         * release = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_DEREF_EX(starter, 1, 0)
        Z_PARAM_ZVAL_DEREF_EX(release, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
    if(starter) {
        ion_promisor_set_initial_callback(sequence, starter);
    }
    if(release) {
        ion_promisor_set_php_cb(&sequence->canceler, pion_cb_create_from_zval(release));
        sequence->flags |= ION_PROMISOR_AUTOCLEAN;
    }
}

METHOD_ARGS_BEGIN(ION_Sequence, __construct, 0)
    METHOD_ARG_CALLBACK(handler, 0, 0)
METHOD_ARGS_END()


/** public function ION\Sequence::__invoke(mixed ...$data) : void */
CLASS_METHOD(ION_Sequence, __invoke) {
    zval * data        = NULL;
    int    count       = 0;
    ion_promisor * seq = ION_THIS_OBJECT(ion_promisor);

    ZEND_PARSE_PARAMETERS_START(0, 255)
        Z_PARAM_VARIADIC('*', data, count);
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(!count) {
        zval nil;
        ZVAL_NULL(&nil);
        ion_promisor_sequence_invoke(seq, &nil);
    } else if(count == 1) {
        ion_promisor_sequence_invoke(seq, data);
    } else {
        ion_promisor_sequence_invoke_args(seq, data, count);
    }
}

METHOD_ARGS_BEGIN(ION_Sequence, __invoke, 1)
   METHOD_ARG(data, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Sequence)
    METHOD(ION_Sequence, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Sequence, __invoke,    ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Sequence) {
    pion_register_extended_class(ION_Sequence, ion_ce_ION_Promise, "ION\\Sequence", ion_sequence_zend_init, CLASS_METHODS(ION_Sequence));
    pion_init_std_object_handlers(ION_Sequence);
    pion_set_object_handler(ION_Sequence, free_obj, ion_promisor_zend_free);
    pion_set_object_handler(ION_Sequence, clone_obj, ion_promisor_zend_clone);
    ion_class_set_offset(ion_oh_ION_Sequence, ion_promisor);

    return SUCCESS;
}