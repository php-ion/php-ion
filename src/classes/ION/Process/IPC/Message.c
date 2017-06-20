#include "ion.h"

zend_object_handlers ion_oh_ION_Process_IPC_Message;
zend_class_entry * ion_ce_ION_Process_IPC_Message;


METHODS_START(methods_ION_Process_IPC_Message)
METHODS_END;


PHP_MINIT_FUNCTION(ION_Process_IPC_Message) {
    ion_register_class(ion_ce_ION_Process_IPC_Message, "ION\\Process\\IPC\\Message", NULL, methods_ION_Process_IPC_Message);
    ion_class_declare_property_null(ion_ce_ION_Process_IPC_Message, "ctx", ZEND_ACC_PUBLIC);
    ion_class_declare_property_string(ion_ce_ION_Process_IPC_Message, "data", "", ZEND_ACC_PUBLIC);
    return SUCCESS;
}
