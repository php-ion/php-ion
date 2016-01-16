#include "../../pion.h"

#define ION_STORAGE_RELEASED  (1<<0)
#define ION_STORAGE_HANDSHAKE (1<<1)

zend_object_handlers ion_oh_ION_Stream_Storage;
zend_class_entry * ion_ce_ION_Stream_Storage;

/** public function ION\Stream\Storage::addStream(ION\Stream $stream) : Sequence  */
CLASS_METHOD(ION_Stream_Storage, addStream) {
    ion_storage * storage = get_this_instance(ion_storage);
    zend_long     flags   = ION_STORAGE_HANDSHAKE;
    zval        * zstream = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(zstream)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ion_storage_add_stream(Z_OBJ_P(getThis()), Z_OBJ_P(zstream));

    if((flags & ION_STORAGE_HANDSHAKE) && storage->handshake_handler && storage->handshake) {
        ion_promisor_sequence_invoke(storage->handshake, zstream);
    } else if((flags & ION_STORAGE_RELEASED)) {

    }
}

METHOD_ARGS_BEGIN(ION_Stream_Storage, addStream, 1)
        METHOD_ARG_OBJECT(stream, ION\\Stream , 0, 0)
METHOD_ARGS_END();

/** public function ION\Stream\Storage::removeStream($peer_name) : Sequence  */
CLASS_METHOD(ION_Stream_Storage, removeStream) {
//    ion_storage * storage = get_this_instance(ion_storage);
    zend_string * peer_name = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(peer_name)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

}

METHOD_ARGS_BEGIN(ION_Stream_Storage, removeStream, 1)
    METHOD_ARG_STRING(peer_name, 0)
METHOD_ARGS_END();

CLASS_METHODS_START(ION_Stream_Storage)
    METHOD(ION_Stream_Storage, addStream, ZEND_ACC_PUBLIC)
    METHOD(ION_Stream_Storage, removeStream, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Stream_Storage) {
    pion_register_extended_class(ION_Stream_Storage, ion_ce_ION_Stream_StorageAbstract, "ION\\Stream\\Storage", ion_storage_init, CLASS_METHODS(ION_Stream_Storage));
    pion_init_std_object_handlers(ION_Stream_Storage);
    pion_set_object_handler(ION_Stream_Storage, free_obj, ion_storage_free);
    pion_set_object_handler(ION_Stream_Storage, clone_obj, NULL);
    return SUCCESS;
}
