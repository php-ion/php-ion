#include "ion.h"

zend_object_handlers ion_oh_ION_Process_ChildProcess;
zend_class_entry * ion_ce_ION_Process_ChildProcess;

zend_object * ion_process_child_init(zend_class_entry * ce) {
    ion_process_child * child = ecalloc(1, sizeof(ion_process_child));
    child->exit_status = -1;
    RETURN_INSTANCE(ION_Process_ChildProcess, child);
}


void ion_process_child_free(zend_object * object) {
    ion_process_child * child = get_object_instance(object, ion_process_child);
    if(child->released) {
        zend_object_release(child->released);
        child->released = NULL;
    }
    if(child->run) {
        zend_object_release(child->run);
        child->run = NULL;
    }
    if(child->ipc) {
        zend_object_release(ION_OBJ(child->ipc));
        child->ipc = NULL;
    }
}

void ion_process_child_spawn(ion_process_child * worker) {
    int pid = 0;
    zval ipc4parent;
    zval ipc4child;
    zval ctx4parent;
    zval res;
    zend_object * promise = worker->run;
    ZVAL_NULL(&res);
//
    if(ion_ipc_create(&ipc4parent, &ipc4child, &ctx4parent, NULL) == FAILURE) {
        return;
    }
    errno = 0;
    pid = fork();
    if(pid == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to spawn worker: %s", strerror(errno));
        return;
    } else if(pid) { // parent
        worker->flags |= ION_WORKER_CHILD | ION_WORKER_STARTED;
        worker->pid = pid;
        zval_ptr_dtor(&ipc4child);
        worker->ipc = get_object_instance(Z_OBJ(ipc4parent), ion_process_ipc);
        ZVAL_OBJ(&worker->ipc->ctx, ION_OBJ(worker));
        zval_add_ref(&worker->ipc->ctx);
        ion_process_add_child(pid, ION_PROC_CHILD_WORKER, ION_OBJ(worker));
        ZVAL_OBJ(&res, ION_OBJ(worker));
    } else { // child
        if(event_reinit(GION(base)) == FAILURE) {
            zend_error(E_NOTICE, "Some events could not be restarted");
        }
        zend_object_release(ION_OBJ(worker));

    }
    ion_promisor_done(promise, &res);
}

/** public function ION\Process\Worker::getStartedTime() : float */
CLASS_METHOD(ION_Process_ChildProcess, getStartedTime) {
    ion_process_child * worker = get_this_instance(ion_process_child);

    double time = worker->started_time.tv_sec;
    time += worker->started_time.tv_usec / 1e6;

    RETURN_DOUBLE(time);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getStartedTime);

/** public function ION\Process\Worker::getPID() : int */
CLASS_METHOD(ION_Process_ChildProcess, getPID) {
    ion_process_child * worker = get_this_instance(ion_process_child);
    RETURN_LONG(worker->pid);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getPID);

/** public function ION\Process\Worker::isAlive() : bool */
CLASS_METHOD(ION_Process_ChildProcess, isAlive) {
    ion_process_child * worker = get_this_instance(ion_process_child);
    if((worker->flags & ION_WORKER_FINISHED) || !(worker->flags & ION_WORKER_STARTED)) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isAlive);

/** public function ION\Process\Worker::isFinished() : bool */
CLASS_METHOD(ION_Process_ChildProcess, isFinished) {
    ion_process_child * worker = get_this_instance(ion_process_child);
    if(worker->flags & ION_WORKER_FINISHED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isFinished);


/** public function ION\Process\Worker::isSignaled() : bool */
CLASS_METHOD(ION_Process_ChildProcess, isSignaled) {
    ion_process_child * worker = get_this_instance(ion_process_child);
    if(worker->flags & ION_WORKER_SIGNALED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isSignaled);

/** public function ION\Process\Worker::isStarted() : int */
CLASS_METHOD(ION_Process_ChildProcess, isStarted) {
    ion_process_child * worker = get_this_instance(ion_process_child);
    if(worker->flags & ION_WORKER_STARTED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, isStarted);

/** public function ION\Process\Worker::getSignal() : int */
CLASS_METHOD(ION_Process_ChildProcess, getSignal) {
    ion_process_child * worker = get_this_instance(ion_process_child);

    RETURN_LONG(worker->signal);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getSignal);

/** public function ION\Process\Worker::getExitStatus() : int */
CLASS_METHOD(ION_Process_ChildProcess, getExitStatus) {
    ion_process_child * worker = get_this_instance(ion_process_child);

    RETURN_LONG(worker->exit_status);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, getExitStatus);

/** public function ION\Process\Worker::released() : ION\Sequence */
CLASS_METHOD(ION_Process_ChildProcess, released) {
    ion_process_child * worker = get_this_instance(ion_process_child);

    if(!worker->released) {
        worker->released = ion_promisor_promise_new(NULL, NULL);
    }
    zend_object_addref(worker->released);
    RETURN_OBJ(worker->released);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, released);


/** public function ION\Process\Worker::run() : ION\Promise */
CLASS_METHOD(ION_Process_ChildProcess, run) {
    ion_process_child * worker = get_this_instance(ion_process_child);
    pion_cb * worker_spawn_method = NULL;

    if(!worker->run) {
        worker->run = ion_promisor_promise_new(NULL, NULL);
    }

    worker_spawn_method = pion_cb_fetch_method("ION\\Process\\Worker", "_spawn");
    ion_deferred_queue_push(worker_spawn_method, Z_OBJ_P(getThis()));

    zend_object_addref(worker->run);
    RETURN_OBJ(worker->run);
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, run);


CLASS_METHOD(ION_Process_ChildProcess, _spawn) {
    ion_process_child_spawn(get_this_instance(ion_process_child));
}

METHOD_WITHOUT_ARGS(ION_Process_ChildProcess, _spawn);

CLASS_METHODS_START(ION_Process_ChildProcess)
    METHOD(ION_Process_ChildProcess, getStartedTime, ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getPID,         ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isAlive,        ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isFinished,     ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isStarted,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, isSignaled,     ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getSignal,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, getExitStatus,  ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, released,       ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, run,            ZEND_ACC_PUBLIC)
    METHOD(ION_Process_ChildProcess, _spawn,         ZEND_ACC_PRIVATE)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Process_ChildProcess) {
    pion_register_class(ION_Process_ChildProcess, "ION\\Process\\ChildProcess", ion_process_child_init, CLASS_METHODS(ION_Process_ChildProcess));
    pion_init_std_object_handlers(ION_Process_ChildProcess);
    pion_set_object_handler(ION_Process_ChildProcess, free_obj, ion_process_child_free);
    pion_set_object_handler(ION_Process_ChildProcess, clone_obj, NULL);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process_ChildProcess) {
    ALLOC_HASHTABLE(GION(workers));
    zend_hash_init(GION(workers), 128, NULL, zval_ptr_dtor_wrapper, 0);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process_ChildProcess) {
    zend_hash_clean(GION(workers));
    zend_hash_destroy(GION(workers));
    FREE_HASHTABLE(GION(workers));

//    if(GION(master)) {
//        zend_object_release(GION(master));
//    }
    return SUCCESS;
}