#include "ion.h"


zend_object_handlers ion_oh_ION_Stream_Client;
zend_class_entry * ion_ce_ION_Stream_Client;

/** public function ION\Stream\Client::addTarget(string $target, string $host, ION\Crypto $encrypt = null) : self  */
CLASS_METHOD(ION_Stream_Client, addTarget) {
//    ion_storage * storage = get_this_instance(ion_storage);
    zend_string * target = NULL;
    zend_string * host = NULL;
    zval        * encrypt = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_STR(target)
        Z_PARAM_STR(host)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(encrypt)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    // TODO
}

METHOD_ARGS_BEGIN(ION_Stream_Client, addTarget, 2)
    METHOD_ARG_STRING(target, 0)
    METHOD_ARG_STRING(host, 0)
    METHOD_ARG_OBJECT(encrypt, ION\\Crypto, 1, 0)
METHOD_ARGS_END();

/** public function ION\Stream\Server::fetchStream(string $target = null) : Deferred  */
CLASS_METHOD(ION_Stream_Client, fetchStream) {
//    ion_storage * storage = get_this_instance(ion_storage);
    zend_string * target = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 0)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    // TODO
}

METHOD_ARGS_BEGIN(ION_Stream_Client, fetchStream, 1)
    METHOD_ARG_STRING(target, 0)
METHOD_ARGS_END();


CLASS_METHODS_START(ION_Stream_Client)
    METHOD(ION_Stream_Client, addTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_Stream_Client, fetchStream, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Stream_Client) {
    pion_register_extended_class(ION_Stream_Client, ion_ce_ION_Stream_StorageAbstract, "ION\\Stream\\Client", ion_storage_init, CLASS_METHODS(ION_Stream_Client));
    pion_init_std_object_handlers(ION_Stream_Client);
    pion_set_object_handler(ION_Stream_Client, free_obj, ion_storage_free);
    pion_set_object_handler(ION_Stream_Client, clone_obj, NULL);
    return SUCCESS;
}
