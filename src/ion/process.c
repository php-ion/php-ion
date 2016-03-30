#include "ion.h"


void ion_process_sigchld(evutil_socket_t signal, short flags, void * arg) {
    IONF("SIGCHLD received: check execs and workers");
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    zend_object * child;

    if(pid <= 0) {
        // check disconnected workers
    } else {
        child = zend_hash_index_find_ptr(GION(childs), (zend_ulong) pid);
        if(child) {
            if(ion_procces_is_exec(child)) {
                ion_process_exec_exit(child, status);
            } else {
                ion_process_worker_exit(child, status);
            }
            zend_hash_index_del(GION(childs), (zend_ulong) pid);
        }
    }
}

void ion_process_add_child(pid_t pid, ion_process_child_type type, zend_object * object) {
    ion_process_child * child = get_object_instance(object, ion_process_child);
    child->flags |= type;

    zend_hash_index_add_ptr(GION(childs), (zend_ulong) pid, object);
}

void ion_process_exec_disconnect(ion_buffer * b, short what, void * ctx) {
    ion_process_child * child = (ion_process_child *)ctx;
    if(what & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
        child->flags |= ION_WORKER_DISCONNECTED;
        zend_hash_index_add_ptr(GION(disconnected), (zend_ulong) child->pid, NULL);
    }
}

void ion_process_exec_exit(zend_object * exec, int status) {
    zval result;
    zend_string * out;
    zend_string * err;
    ion_process_exec * e = get_object_instance(exec, ion_process_exec);


    ZVAL_OBJ(&result, exec);
    zval_add_ref(&result);

    pion_update_property_long(ION_Process_ExecResult, &result, "pid", e->pid);
    if(WIFSIGNALED(status)) {
        pion_update_property_bool(ION_Process_ExecResult, &result, "signaled", 1);
        pion_update_property_long(ION_Process_ExecResult, &result, "signal", WTERMSIG(status));
        pion_update_property_long(ION_Process_ExecResult, &result, "status", status);
    } else {
        pion_update_property_long(ION_Process_ExecResult, &result, "status", WEXITSTATUS(status));
    }
    out = ion_buffer_read_all(e->out);
    if(out) {
        pion_update_property_str(ION_Process_ExecResult, &result, "stdout", out);
        zend_string_release(out);
    }
    err = ion_buffer_read_all(e->err);
    if(err) {
        pion_update_property_str(ION_Process_ExecResult, &result, "stderr", err);
        zend_string_release(err);
    }
    ion_promisor_done(e->deferred, &result);
    zend_object_release(e->deferred);
    e->deferred = NULL;
    zval_ptr_dtor(&result);
}

void ion_process_worker_exit(zend_object * w, int status) {
    ion_process_worker * worker = get_object_instance(w, ion_process_worker);
    if(WIFSIGNALED(status)) {
        worker->signal = WTERMSIG(status);
        worker->exit_status = status;
        worker->flags |= ION_WORKER_SIGNALED;
    } else if(WIFSTOPPED(status)) {
        // unreachable
    } else {
        worker->exit_status = WEXITSTATUS(status);
        if(worker->exit_status) {
            worker->flags |= ION_WORKER_FAILED;
        } else {
            worker->flags |= ION_WORKER_DONE;
        }
    }
    if(worker->on_exit) {
        zval container;
        ZVAL_OBJ(&container, ION_OBJ(worker));
        ion_promisor_sequence_invoke(worker->on_exit, &container);
    }
    if(worker->buffer) {
        bufferevent_disable(worker->buffer, EV_READ | EV_WRITE);
        bufferevent_free(worker->buffer);
        worker->buffer = NULL;
    }
    if(worker->flags & ION_WORKER_CHILD) {
        zend_hash_index_del(GION(workers), (zend_ulong) worker->pid);
    }
}

void ion_process_exec_dtor(zend_object * exec) {
    zend_object_release(exec);
}

void ion_process_worker_dtor(zend_object * worker) {
    ion_process_worker * w = get_object_instance(worker, ion_process_worker);
    w->flags |= ION_WORKER_ABORT;
    zend_object_release(worker);
}


void ion_process_child_dtor(zval * pz) {
    zend_object * child = Z_OBJ_P(pz);

    if(ion_procces_is_exec(child)) {
        ion_process_exec_dtor(child);
    } else {
        ion_process_worker_dtor(child);
    }
}