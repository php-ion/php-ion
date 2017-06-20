#include "ion.h"

zend_object_handlers ion_oh_ION_Process_ChildProcess;
zend_class_entry * ion_ce_ION_Process_ChildProcess;

zend_object * ion_process_child_init(zend_class_entry * ce) {
    ion_process_child * child = ion_alloc_object(ce, ion_process_child);
    child->exit_status = -1;
    child->ppid = getpid();

    return ion_init_object(ION_OBJECT_ZOBJ(child), ce, &ion_oh_ION_Process_ChildProcess);
}


void ion_process_child_free(zend_object * object) {
    ion_process_child * child = ION_ZOBJ_OBJECT(object, ion_process_child);
    zend_object_std_dtor(object);
    if(child->prom_exit) {
        ion_object_release(child->prom_exit);
        child->prom_exit = NULL;
    }
    if(child->prom_started) {
        ion_object_release(child->prom_started);
        child->prom_started = NULL;
    }
    if(child->ipc_parent) {
        ion_object_release(child->ipc_parent);
        child->ipc_parent = NULL;
    }
    if(child->ipc_child) {
        ion_object_release(child->ipc_child);
        child->ipc_child = NULL;
    }
    if(child->on_start) {
        pion_cb_release(child->on_start);
        child->on_start = NULL;
    }
}

void ion_process_child_spawn(ion_process_child * worker) {
    int pid = 0;
    zval w;

    if(worker->ppid != getpid()) {
        return;
    }

    errno = 0;
    pid = fork();
    if(pid == -1) {
        zend_throw_exception_ex(ion_ce_ION_ProcessException, 0, ERR_ION_PROCESS_SPAWN_FAIL, strerror(errno));
        return;
    } else if(pid) { // in parent
        worker->flags |= ION_PROCESS_STARTED;
        worker->pid = pid;
        ion_object_release(worker->ipc_child);
        worker->ipc_child = NULL;
        bufferevent_enable(worker->ipc_parent->buffer, EV_READ | EV_WRITE);
        ion_object_addref(worker);
        ZVAL_OBJ(&w, ION_OBJECT_ZOBJ(worker));
        zend_hash_index_add(GION(proc_childs), (zend_ulong) pid, &w);
        if(worker->prom_started) {
            // use "weak" ref
            zval chld;
            ZVAL_NEW_EMPTY_REF(&chld);
            ZVAL_OBJ(Z_REFVAL(chld), ION_OBJECT_ZOBJ(worker));
            ion_promisor_done(worker->prom_started, &chld);
            zval_ptr_dtor(&chld);
        }
    } else { // in child
        zval ipc;
        if(event_reinit(GION(base)) == FAILURE) {
            zend_error(E_WARNING, ERR_ION_REINIT_FAILED);
        }

        if(worker->on_start) {
            ZVAL_OBJ(&ipc, ION_OBJECT_ZOBJ(worker->ipc_child));
            pion_cb_call_with_1_arg(worker->on_start, &ipc);

        }
        ion_object_release(worker->ipc_child);
        if(GION(parent_ipc)) {
            ion_object_release(GION(parent_ipc));
            GION(parent_ipc) = worker->ipc_parent;
            ion_object_addref(GION(parent_ipc));
        }
        bufferevent_enable(worker->ipc_child->buffer, EV_READ | EV_WRITE);
        worker->ipc_child = NULL;
        ion_object_release(worker);

    }
}


void ion_process_child_exit(zend_object * w, int status) {
    ion_process_child * child = ION_ZOBJ_OBJECT(w, ion_process_child);

    if(child->ppid != getpid()) {
        return;
    }

    if(WIFSIGNALED(status)) {
        child->signal = WTERMSIG(status);
        child->exit_status = status;
        child->flags |= ION_PROCESS_SIGNALED;
    } else if(WIFSTOPPED(status)) {
        // unreachable
    } else {
        child->exit_status = WEXITSTATUS(status);
        if(child->exit_status) {
            child->flags |= ION_PROCESS_FAILED;
        } else {
            child->flags |= ION_PROCESS_DONE;
        }
    }
    if(child->prom_exit) {
        // use "weak" ref
        zval chld;
        ZVAL_NEW_EMPTY_REF(&chld);
        ZVAL_OBJ(Z_REFVAL(chld), w);
        ion_promisor_done(child->prom_exit, &chld);
        zval_ptr_dtor(&chld);
    }
}

/** public function ION\Process\ChildProcess::__construct() : void */
CLASS_METHOD(ION_Process_ChildProcess, __construct) {
    ion_process_child * child = ION_THIS_OBJECT(ion_process_child);
    zval ipc_parent;
    zval ipc_child;
    zval context;

    ZVAL_NEW_EMPTY_REF(&context);
    ZVAL_OBJ(Z_REFVAL(context), Z_OBJ_P(getThis()));

    int r = ion_ipc_create(&ipc_parent, &ipc_child, &context, NULL, 0);
    zval_ptr_dtor(&context);

    if(r == FAILURE) {
        zend_throw_exception(ion_ce_ION_RuntimeException, ERR_ION_PROCESS_IPC_FAIL, 0);
        return;
    }

    child->ipc_parent = ION_ZVAL_OBJECT(ipc_parent, ion_process_ipc);
    child->ipc_child  = ION_ZVAL_OBJECT(ipc_child, ion_process_ipc);
    bufferevent_disable(child->ipc_parent->buffer, EV_READ | EV_WRITE);
    bufferevent_disable(child->ipc_child->buffer, EV_READ | EV_WRITE);

}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, __construct);

/** public function ION\Process\ChildProcess::getStartedTime() : float */
CLASS_METHOD(ION_Process_ChildProcess, getStartedTime) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);

    double time = worker->started_time.tv_sec;
    time += worker->started_time.tv_usec / 1e6;

    RETURN_DOUBLE(time);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getStartedTime);

/** public function ION\Process\ChildProcess::getPID() : int */
CLASS_METHOD(ION_Process_ChildProcess, getPID) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);
    RETURN_LONG(worker->pid);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getPID);

/** public function ION\Process\ChildProcess::isAlive() : bool */
CLASS_METHOD(ION_Process_ChildProcess, isAlive) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);
    if((worker->flags & ION_PROCESS_FINISHED) || !(worker->flags & ION_PROCESS_STARTED)) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isAlive);

/** public function ION\Process\ChildProcess::isFinished() : bool */
CLASS_METHOD(ION_Process_ChildProcess, isFinished) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);
    if(worker->flags & ION_PROCESS_FINISHED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isFinished);


/** public function ION\Process\ChildProcess::isSignaled() : bool */
CLASS_METHOD(ION_Process_ChildProcess, isSignaled) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);
    if(worker->flags & ION_PROCESS_SIGNALED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isSignaled);

/** public function ION\Process\ChildProcess::isStarted() : int */
CLASS_METHOD(ION_Process_ChildProcess, isStarted) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);
    if(worker->flags & ION_PROCESS_STARTED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isStarted);

/** public function ION\Process\ChildProcess::getSignal() : int */
CLASS_METHOD(ION_Process_ChildProcess, getSignal) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);

    RETURN_LONG(worker->signal);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getSignal);

/** public function ION\Process\ChildProcess::getExitStatus() : int */
CLASS_METHOD(ION_Process_ChildProcess, getExitStatus) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);

    RETURN_LONG(worker->exit_status);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getExitStatus);

/** public function ION\Process\ChildProcess::getIPC() : IPC */
CLASS_METHOD(ION_Process_ChildProcess, getIPC) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);

    if(worker->ipc_parent) {
        ion_object_addref(worker->ipc_parent);
        RETURN_ION_OBJ(worker->ipc_parent);
    } else {
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getIPC);

/** public function ION\Process\ChildProcess::whenExit() : ION\Promise */
CLASS_METHOD(ION_Process_ChildProcess, whenExit) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);

    if(!worker->prom_exit) {
        worker->prom_exit = ion_promisor_promise_new(NULL, NULL);
    }
    ion_object_addref(worker->prom_exit);
    RETURN_ION_OBJ(worker->prom_exit);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, whenExit);

/** public function ION\Process\ChildProcess::whenStarted() : ION|Promise */
CLASS_METHOD(ION_Process_ChildProcess, whenStarted) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);

    if(!worker->prom_started) {
        worker->prom_started = ion_promisor_promise_new(NULL, NULL);
    }
    ion_object_addref(worker->prom_started);
    RETURN_ION_OBJ(worker->prom_started);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, whenStarted);


/** public function ION\Process\ChildProcess::start(callable $callback) : self */
CLASS_METHOD(ION_Process_ChildProcess, start) {
    ion_process_child * worker = ION_THIS_OBJECT(ion_process_child);
    pion_cb * worker_spawn_method = NULL;
    zval * callback = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL_EX(callback, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(worker->on_start) {
        pion_cb_release(worker->on_start);
    }
    worker->on_start = pion_cb_create_from_zval(callback);

    worker_spawn_method = pion_cb_fetch_method("ION\\Process\\ChildProcess", "_spawn");
    ion_deferred_queue_push(worker_spawn_method, Z_OBJ_P(getThis()));

    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, start);


CLASS_METHOD(ION_Process_ChildProcess, _spawn) {
    ion_process_child_spawn(ION_THIS_OBJECT(ion_process_child));
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, _spawn);

METHODS_START(methods_ION_Process_ChildProcess)
    METHOD(ION_Process_ChildProcess, __construct,  ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getStartedTime, ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getPID,         ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isAlive,        ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isFinished,     ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isStarted,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isSignaled,     ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getSignal,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getExitStatus,  ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getIPC,         ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, whenExit,       ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, whenStarted,    ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, start,          ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, _spawn,         ZEND_ACC_PRIVATE)
METHODS_END;

PHP_MINIT_FUNCTION(ION_Process_ChildProcess) {
    ion_register_class(ion_ce_ION_Process_ChildProcess, "ION\\Process\\ChildProcess", ion_process_child_init, methods_ION_Process_ChildProcess);
    ion_init_object_handlers(ion_oh_ION_Process_ChildProcess);
    ion_oh_ION_Process_ChildProcess.free_obj = ion_process_child_free;
    ion_oh_ION_Process_ChildProcess.clone_obj = NULL;
    ion_oh_ION_Process_ChildProcess.offset = ion_offset(ion_process_child);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process_ChildProcess) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process_ChildProcess) {
    return SUCCESS;
}