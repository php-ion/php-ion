#include "ion.h"

zend_object_handlers ion_oh_ION_Process_IPC_Message;
zend_class_entry * ion_ce_ION_Process_IPC_Message;


CLASS_METHODS_START(ION_Process_IPC_Message)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Process_IPC_Message) {
    PION_REGISTER_DEFAULT_CLASS(ION_Process_IPC_Message, "ION\\Process\\IPC\\Message");
    zend_declare_property_null(ion_ce_ION_Process_IPC_Message, STRARGS("ctx"), ZEND_ACC_PUBLIC);
    zend_declare_property_string(ion_ce_ION_Process_IPC_Message, STRARGS("data"), "", ZEND_ACC_PUBLIC);
    return SUCCESS;
}
