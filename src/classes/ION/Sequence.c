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
        if(!ion_promisor_set_initial_callback(sequence, starter)) {
            zend_throw_exception(ion_ce_InvalidArgumentException, "Invalid callback", 0);
        }
    }
    if(release) {
        ion_promisor_set_php_cb(&sequence->canceler, pion_cb_create_from_zval(release));
        sequence->flags |= ION_PROMISOR_AUTOCLEAN;
    }
}

METHOD_ARGS_BEGIN(ION_Sequence, __construct, 0)
    ARGUMENT(handler, IS_MIXED)
METHOD_ARGS_END()


/** public function ION\Sequence::__invoke(mixed ...$data) : void */
CLASS_METHOD(ION_Sequence, __invoke) {
    zval * data        = NULL;
    int    count       = 0;
    ion_promisor * seq = ION_THIS_OBJECT(ion_promisor);
    ion_promisor * res = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 255)
        Z_PARAM_VARIADIC('*', data, count);
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(!count) {
        res = ion_promisor_done_null_ex(seq);
    } else if(count == 1) {
        res = ion_promisor_done_ex(seq, data);
    } else {
        res = ion_promisor_done_args(seq, data, count);
    }

    if(res) {
        RETURN_ION_OBJ(res);
    } else {
        zend_throw_exception(ion_ce_ION_RuntimeException, "", 0);
    }

}

METHOD_ARGS_BEGIN(ION_Sequence, __invoke, 1)
   ARGUMENT(data, IS_MIXED | ARG_IS_VARIADIC)
METHOD_ARGS_END()


METHODS_START(methods_ION_Sequence)
    METHOD(ION_Sequence, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Sequence, __invoke,    ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_Sequence) {
    ion_register_class_ex(&ion_ce_ION_Sequence, ion_ce_ION_Promise, "ION\\Sequence", ion_sequence_zend_init, methods_ION_Sequence);
    ion_init_object_handlers(ion_oh_ION_Sequence);
    ion_oh_ION_Sequence.free_obj = ion_promisor_zend_free;
    ion_oh_ION_Sequence.clone_obj = ion_promisor_zend_clone;
    ion_oh_ION_Sequence.offset = ion_offset(ion_promisor);

    return SUCCESS;
}