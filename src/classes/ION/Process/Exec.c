#include "ion.h"

zend_object_handlers ion_oh_ION_Process_Exec;
zend_class_entry * ion_ce_ION_Process_Exec;

zend_object * ion_process_exec_init(zend_class_entry * ce) {
//    zval tmp;
    ion_process_exec * exec = ion_alloc_object(ce, ion_process_exec);
//    pion_update_property_long(ION_Process_Exec, &result, "pid", e->pid);
//    if(WIFSIGNALED(status)) {
//        pion_update_property_bool(ION_Process_Exec, &result, "signaled", 1);
//        pion_update_property_long(ION_Process_Exec, &result, "signal", WTERMSIG(status));
//        pion_update_property_long(ION_Process_Exec, &result, "status", status);
//    } else {
//        pion_update_property_long(ION_Process_Exec, &result, "status", WEXITSTATUS(status));
//    }
//    out = ion_buffer_read_all(e->out);
//    if(out) {
//        pion_update_property_str(ION_Process_Exec, &result, "stdout", out);
//    }
//    err = ion_buffer_read_all(e->err);
//    if(err) {
//        pion_update_property_str(ION_Process_Exec, &result, "stderr", err);
//    }
//    zend_object_std_init(ION_OBJ(exec), ce);
//    object_properties_init(ION_OBJ(exec), ce);
    return ion_init_object(ION_OBJECT_ZOBJ(exec), ce, &ion_oh_ION_Process_Exec);
//    ZVAL_OBJ(&tmp, ION_OBJ(exec));
//    ION_OBJ_HANDLERS(exec) = &ion_oh_ION_Process_Exec;
//
//    pion_update_property_string(ION_Process_Exec, &tmp, "command", "");
//    pion_update_property_long(ION_Process_Exec,   &tmp, "pid", 0);
//    pion_update_property_bool(ION_Process_Exec,   &tmp, "signaled", 0);
//    pion_update_property_long(ION_Process_Exec,   &tmp, "signal", 0);
//    pion_update_property_long(ION_Process_Exec,   &tmp, "status", -1);
//    pion_update_property_string(ION_Process_Exec, &tmp, "stdout", "");
//    pion_update_property_string(ION_Process_Exec, &tmp, "stderr", "");
//
//    return ION_OBJ(exec);
}


void ion_process_exec_free(zend_object * object) {
    ion_process_exec * exec_result = ION_ZOBJ_OBJECT(object, ion_process_exec);
    zend_object_std_dtor(object);
    if(exec_result->deferred) {
        ion_object_release(exec_result->deferred);
    }
    if(exec_result->in) {
        bufferevent_free(exec_result->in);
    }
    if(exec_result->out) {
        bufferevent_free(exec_result->out);
    }
    if(exec_result->err) {
        bufferevent_free(exec_result->err);
    }
}


METHODS_START(methods_ION_Process_Exec)
METHODS_END;


PHP_MINIT_FUNCTION(ION_Process_Exec) {
    ion_register_class(ion_ce_ION_Process_Exec, "ION\\Process\\Exec", ion_process_exec_init, methods_ION_Process_Exec);
    ion_init_object_handlers(ion_oh_ION_Process_Exec);
    ion_oh_ION_Process_Exec.free_obj = ion_process_exec_free;
    ion_oh_ION_Process_Exec.offset = ion_offset(ion_process_exec);

    ion_class_declare_property_string(ion_ce_ION_Process_Exec, "command", "", ZEND_ACC_PUBLIC);
    ion_class_declare_property_long(ion_ce_ION_Process_Exec, "pid", 0, ZEND_ACC_PUBLIC);
    ion_class_declare_property_bool(ion_ce_ION_Process_Exec, "signaled", 0, ZEND_ACC_PUBLIC);
    ion_class_declare_property_long(ion_ce_ION_Process_Exec, "signal", 0, ZEND_ACC_PUBLIC);
    ion_class_declare_property_long(ion_ce_ION_Process_Exec, "status", 0, ZEND_ACC_PUBLIC);
    ion_class_declare_property_string(ion_ce_ION_Process_Exec, "stdout", "", ZEND_ACC_PUBLIC);
    ion_class_declare_property_string(ion_ce_ION_Process_Exec, "stderr", "", ZEND_ACC_PUBLIC);

    return SUCCESS;
}
