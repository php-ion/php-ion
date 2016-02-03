#include <ion_core.h>

zend_object_handlers ion_oh_ION_Sequence;
zend_class_entry * ion_ce_ION_Sequence;
zend_object_handlers ion_oh_ION_Sequence_Quit;
zend_class_entry * ion_ce_ION_Sequence_Quit;


zend_object * ion_sequence_init(zend_class_entry * ce) {
    ion_promisor * promise = emalloc(sizeof(ion_promisor));
    memset(promise, 0, sizeof(ion_promisor));
    promise->flags |= ION_PROMISOR_TYPE_PROMISE | ION_PROMISOR_TYPE_SEQUENCE | ION_PROMISOR_PROTOTYPE;
    RETURN_INSTANCE(ION_Sequence, promise);
}

/** public function ION\Sequence::quit() : ION\Sequence\Quit */
CLASS_METHOD(ION_Sequence, quit) {
    zend_object * quit = GION(quit_marker);
    zend_object_addref(quit);
    RETURN_OBJ(quit);
}

METHOD_WITHOUT_ARGS(ION_Sequence, quit);

/** public function ION\Sequence::__construct(callable $handler) : self */
CLASS_METHOD(ION_Sequence, __construct) {
    zval * handler = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(handler, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
    ion_promisor_set_initial_callback(Z_OBJ_P(getThis()), handler);
}

METHOD_ARGS_BEGIN(ION_Sequence, __construct, 0)
    METHOD_ARG_CALLBACK(handler, 0, 0)
METHOD_ARGS_END()


/** public function ION\Sequence::__invoke(mixed ...$data) : void */
CLASS_METHOD(ION_Sequence, __invoke) {
    zval * data = NULL;
    int    count = 0;

    ZEND_PARSE_PARAMETERS_START(0, 255)
        Z_PARAM_VARIADIC('*', data, count);
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(!count) {
        zval nil;
        ZVAL_NULL(&nil);
        ion_promisor_sequence_invoke(Z_OBJ_P(getThis()), &nil);
    } else if(count == 1) {
        ion_promisor_sequence_invoke(Z_OBJ_P(getThis()), data);
    } else {
        ion_promisor_sequence_invoke_args(Z_OBJ_P(getThis()), data, count);
    }
}

METHOD_ARGS_BEGIN(ION_Sequence, __invoke, 1)
   METHOD_ARG(data, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Sequence)
    METHOD(ION_Sequence, quit,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Sequence, __construct, ZEND_ACC_PUBLIC)
    METHOD(ION_Sequence, __invoke,    ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

CLASS_METHODS_START(ION_Sequence_Quit)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Sequence) {
    PION_REGISTER_DEFAULT_CLASS(ION_Sequence_Quit, "ION\\Sequence\\Quit");
    pion_register_extended_class(ION_Sequence, ion_class_entry(ION_Promise), "ION\\Sequence", ion_sequence_init, CLASS_METHODS(ION_Sequence));
    pion_init_std_object_handlers(ION_Sequence);
    pion_set_object_handler(ION_Sequence, free_obj, ion_promisor_free);
    pion_set_object_handler(ION_Sequence, clone_obj, ion_promisor_clone_obj);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Sequence) {
    GION(quit_marker) = pion_new_object_arg_0(ion_ce_ION_Sequence_Quit);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Sequence) {
    zend_object_release(GION(quit_marker));
    return SUCCESS;
}
