#include "ion.h"

zend_object_handlers ion_oh_ION_Process_Exec;
zend_class_entry * ion_ce_ION_Process_Exec;

zend_object * ion_process_exec_init(zend_class_entry * ce) {
    zval tmp;
    ion_process_exec * exec = ecalloc(1, sizeof(ion_process_exec));
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
    zend_object_std_init(ION_OBJ(exec), ce);
    object_properties_init(ION_OBJ(exec), ce);
    ZVAL_OBJ(&tmp, ION_OBJ(exec));
    ION_OBJ_HANDLERS(exec) = &ion_oh_ION_Process_Exec;

    pion_update_property_string(ION_Process_Exec, &tmp, "command", "");
    pion_update_property_long(ION_Process_Exec,   &tmp, "pid", 0);
    pion_update_property_bool(ION_Process_Exec,   &tmp, "signaled", 0);
    pion_update_property_long(ION_Process_Exec,   &tmp, "signal", 0);
    pion_update_property_long(ION_Process_Exec,   &tmp, "status", -1);
    pion_update_property_string(ION_Process_Exec, &tmp, "stdout", "");
    pion_update_property_string(ION_Process_Exec, &tmp, "stderr", "");

    return ION_OBJ(exec);
}


void ion_process_exec_free(zend_object * object) {
    ion_process_exec * exec_result = get_object_instance(object, ion_process_exec);
    zend_object_std_dtor(object);
    if(exec_result->deferred) {
        zend_object_release(exec_result->deferred);
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


CLASS_METHODS_START(ION_Process_Exec)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Process_Exec) {
    pion_register_class(ION_Process_Exec, "ION\\Process\\Exec", ion_process_exec_init, CLASS_METHODS(ION_Process_Exec));
    pion_init_std_object_handlers(ION_Process_Exec);
    pion_set_object_handler(ION_Process_Exec, free_obj, ion_process_exec_free);
    return SUCCESS;
}
