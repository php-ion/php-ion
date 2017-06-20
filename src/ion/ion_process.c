#include "ion.h"


void ion_process_sigchld(evutil_socket_t signal, short flags, void * arg) {
    ION_CB_BEGIN();
    IONF("SIGCHLD received: check execs and childs");
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    zval * found = NULL;
    zend_object * proc = NULL;

    while(pid > 0) {
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

        pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    }
    ION_CB_END();
}

void ion_process_exec_exit(zend_object * exec, int status) {
    zval result;
    zend_string * out;
    zend_string * err;
    ion_process_exec * e = ION_ZOBJ_OBJECT(exec, ion_process_exec);


    ZVAL_OBJ(&result, exec);
    zval_add_ref(&result);

    ion_update_property_long(ion_ce_ION_Process_Exec, &result, "pid", e->pid);
    if(WIFSIGNALED(status)) {
        ion_update_property_bool(ion_ce_ION_Process_Exec, &result, "signaled", 1);
        ion_update_property_long(ion_ce_ION_Process_Exec, &result, "signal", WTERMSIG(status));
        ion_update_property_long(ion_ce_ION_Process_Exec, &result, "status", status);
    } else {
        ion_update_property_long(ion_ce_ION_Process_Exec, &result, "status", WEXITSTATUS(status));
    }
    out = ion_buffer_read_all(e->out);
    if(out) {
        ion_update_property_str(ion_ce_ION_Process_Exec, &result, "stdout", out);
        zend_string_release(out);
    }
    err = ion_buffer_read_all(e->err);
    if(err) {
        ion_update_property_str(ion_ce_ION_Process_Exec, &result, "stderr", err);
        zend_string_release(err);
    }
    ion_promisor_done(e->deferred, &result);
    ion_object_release(e->deferred);
    e->deferred = NULL;
    zval_ptr_dtor(&result);
}

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
        zval message;
        object_init_ex(&message, ion_ce_ION_Process_IPC_Message);
        zval_add_ref(&ipc->ctx);
        zend_update_property_str(ion_ce_ION_Process_IPC_Message, &message, STRARGS("data"), zend_string_copy(ipc->frame_body));
        zend_update_property(ion_ce_ION_Process_IPC_Message, &message, STRARGS("context"), &ipc->ctx);

        ion_promisor_done(ipc->on_message, &message);

        zval_ptr_dtor(&ipc->ctx);
        zend_string_release(ipc->frame_body);
        zval_ptr_dtor(&message);
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