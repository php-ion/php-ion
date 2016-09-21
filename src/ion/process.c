#include "ion.h"


void ion_process_sigchld(evutil_socket_t signal, short flags, void * arg) {
    ION_CB_BEGIN();
    IONF("SIGCHLD received: check execs and workers");
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    zval * found = NULL;
    zend_object * proc = NULL;

    if(pid <= 0) {
        // check disconnected childs?
    } else {
        found = zend_hash_index_find(GION(proc_childs), (zend_ulong) pid);
        if(found) {
            proc = Z_OBJ_P(found);
            zend_object_addref(proc);
            zend_hash_index_del(GION(proc_childs), (zend_ulong) pid);
            ion_process_child_exit(proc, status);
            zend_object_release(proc);
        } else {
            found = zend_hash_index_find(GION(proc_execs), (zend_ulong) pid);
            if(found) {
                proc = Z_OBJ_P(found);
                zend_object_addref(proc);
                zend_hash_index_del(GION(proc_execs), (zend_ulong) pid);
                ion_process_exec_exit(proc, status);
                zend_object_release(proc);
            }
        }

    }
    ION_CB_END();
}

void ion_process_add_subprocess(pid_t pid, enum ion_process_flags type, zend_object * object) {
    ion_process_child * child = get_object_instance(object, ion_process_child);
    child->flags |= type;

    zend_hash_index_add_ptr(GION(proc_childs), (zend_ulong) pid, object);
}

void ion_process_exec_disconnect(ion_buffer * b, short what, void * ctx) {
    ion_process_child * child = (ion_process_child *)ctx;
    if(what & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
//        child->flags |= ION_PROCESS_DISCONNECTED;
        zend_hash_index_add_ptr(GION(proc_childs), (zend_ulong) child->pid, NULL);
    }
}

void ion_process_exec_exit(zend_object * exec, int status) {
    zval result;
    zend_string * out;
    zend_string * err;
    ion_process_exec * e = get_object_instance(exec, ion_process_exec);


    ZVAL_OBJ(&result, exec);
    zval_add_ref(&result);

    pion_update_property_long(ION_Process_Exec, &result, "pid", e->pid);
    if(WIFSIGNALED(status)) {
        pion_update_property_bool(ION_Process_Exec, &result, "signaled", 1);
        pion_update_property_long(ION_Process_Exec, &result, "signal", WTERMSIG(status));
        pion_update_property_long(ION_Process_Exec, &result, "status", status);
    } else {
        pion_update_property_long(ION_Process_Exec, &result, "status", WEXITSTATUS(status));
    }
    out = ion_buffer_read_all(e->out);
    if(out) {
        pion_update_property_str(ION_Process_Exec, &result, "stdout", out);
        zend_string_release(out);
    }
    err = ion_buffer_read_all(e->err);
    if(err) {
        pion_update_property_str(ION_Process_Exec, &result, "stderr", err);
        zend_string_release(err);
    }
    ion_promisor_done(e->deferred, &result);
    zend_object_release(e->deferred);
    e->deferred = NULL;
    zval_ptr_dtor(&result);
}


//void ion_process_exec_dtor(zend_object * exec) {
//    zend_object_release(exec);
//}
//
//void ion_process_worker_dtor(zend_object * worker) {
////    ion_process_worker * w = get_object_instance(worker, ion_process_worker);
////    w->flags |= ION_PROCESS_ABORT;
//    zend_object_release(worker);
//}


//void ion_process_child_dtor(zval * pz) {
//    zend_object * child = Z_OBJ_P(pz);
//
//    if(ion_process_is_exec(child)) {
//        ion_process_exec_dtor(child);
//    } else {
//        ion_process_worker_dtor(child);
//    }
//}

/* IPC */
//
//int ion_process_ipc_buffer(ion_buffer ** one, ion_buffer ** two, void * ctx) {
//
//}
//
//
int ion_process_ipc_message_begin(websocket_parser * parser) {
    ion_process_ipc * ipc = parser->data;

    if(parser->length) {
        ipc->frame_body = zend_string_alloc(parser->length, 0);
    }
    return 0;
}

int ion_process_ipc_message_body(websocket_parser * parser, const char * at, size_t length) {
    ion_process_ipc * ipc = parser->data;

    memcpy(&ipc->frame_body->val[parser->offset], at, length);
    return 0;
}

int ion_process_ipc_message_end(websocket_parser * parser) {
    ion_process_ipc * ipc = parser->data;

    if(ipc->on_message) {
        if (ZSTR_LEN(ipc->frame_body) > 0) {
            ZSTR_VAL(ipc->frame_body)[ZSTR_LEN(ipc->frame_body)] = '\0';
        }
        zval c;
        ZVAL_STR(&c, ipc->frame_body);
        zval_add_ref(&c);
        ion_promisor_sequence_invoke(ipc->on_message, &c);
        zval_ptr_dtor(&c);
    }
    zend_string_release(ipc->frame_body);
    return 0;
}
//
//
void ion_process_ipc_incoming(ion_buffer * bev, void * ctx) {
    ION_CB_BEGIN();
    ion_process_ipc * ipc = ctx;
    websocket_parser_settings parser_settings;
    parser_settings.on_frame_header = ion_process_ipc_message_begin;
    parser_settings.on_frame_body   = ion_process_ipc_message_body;
    parser_settings.on_frame_end    = ion_process_ipc_message_end;

    zend_string * data = ion_buffer_read_all(ipc->buffer);
    websocket_parser_execute(ipc->parser, &parser_settings, data->val, data->len);
    zend_string_release(data);
    ION_CB_END();
}