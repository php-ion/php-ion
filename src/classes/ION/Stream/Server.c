#include <ion_core.h>


zend_object_handlers ion_oh_ION_Stream_Server;
zend_class_entry * ion_ce_ION_Stream_Server;

/** public function ION\Stream\Server::listen(string $host, int $backlog = -1) : ION\Listener  */
CLASS_METHOD(ION_Stream_Server, listen) {
//    ion_storage * storage = get_this_instance(ion_storage);
    zend_string * host = NULL;
    zend_long     backlog = -1;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(host)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(backlog)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    // TODO
}

METHOD_ARGS_BEGIN(ION_Stream_Server, listen, 1)
    METHOD_ARG_STRING(host, 0)
    METHOD_ARG_STRING(backlog, 0)
METHOD_ARGS_END();

/** public function ION\Stream\Server::shutdownListener(string $host) : ION\Listener  */
CLASS_METHOD(ION_Stream_Server, shutdownListener) {
//    ion_storage * storage = get_this_instance(ion_storage);
    zend_string * host = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(host)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    // TODO
}

METHOD_ARGS_BEGIN(ION_Stream_Server, shutdownListener, 1)
    METHOD_ARG_STRING(host, 0)
METHOD_ARGS_END();


/** public function ION\Stream\Server::enable() : self  */
CLASS_METHOD(ION_Stream_Server, enable) {
//    ion_storage * storage = get_this_instance(ion_storage);

    // TODO

    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream_Server, enable)

/** public function ION\Stream\Server::disable() : self  */
CLASS_METHOD(ION_Stream_Server, disable) {
//    ion_storage * storage = get_this_instance(ion_storage);

    // TODO

    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream_Server, disable)

CLASS_METHODS_START(ION_Stream_Server)
    METHOD(ION_Stream_Server, listen,           ZEND_ACC_PUBLIC)
    METHOD(ION_Stream_Server, shutdownListener, ZEND_ACC_PUBLIC)
    METHOD(ION_Stream_Server, enable,           ZEND_ACC_PUBLIC)
    METHOD(ION_Stream_Server, disable,          ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Stream_Server) {
    pion_register_extended_class(ION_Stream_Server, ion_ce_ION_Stream_StorageAbstract, "ION\\Stream\\Server", ion_storage_init, CLASS_METHODS(ION_Stream_Server));
    pion_init_std_object_handlers(ION_Stream_Server);
    pion_set_object_handler(ION_Stream_Server, free_obj, ion_storage_free);
    pion_set_object_handler(ION_Stream_Server, clone_obj, NULL);
    return SUCCESS;
}
