#include "ion.h"

zend_object_handlers ion_oh_ION_Process_Worker;
zend_class_entry * ion_ce_ION_Process_Worker;
websocket_parser_settings parser_settings;

pion_cb * worker_spawn_method = NULL;

zend_object * ion_process_worker_init(zend_class_entry * ce) {
    ion_process_worker * worker = ecalloc(1, sizeof(ion_process_worker));
    worker->exit_status = -1;
    RETURN_INSTANCE(ION_Process_Worker, worker);
}


void ion_process_worker_free(zend_object * object) {
    ion_process_worker * worker = get_object_instance(object, ion_process_worker);
    if(worker->buffer) {
        bufferevent_disable(worker->buffer, EV_READ | EV_WRITE);
        bufferevent_free(worker->buffer);
        worker->buffer = NULL;
    }
    if(worker->on_message) {
        zend_object_release(worker->on_message);
        worker->on_message = NULL;
    }
    if(worker->on_exit) {
        zend_object_release(worker->on_exit);
        worker->on_exit = NULL;
    }
    if(worker->on_connect) {
        zend_object_release(worker->on_connect);
        worker->on_connect = NULL;
    }
    if(worker->on_disconnect) {
        zend_object_release(worker->on_disconnect);
        worker->on_disconnect = NULL;
    }
    if(worker->parser) {
        efree(worker->parser);
        worker->parser = NULL;
    }
    if(worker->cb) {
        pion_cb_free(worker->cb);
        worker->cb = NULL;
    }
}

int ion_process_worker_message_begin(websocket_parser * parser) {
    if(parser->flags & WS_OP_CLOSE) {

    }
    return 0;
}

int ion_process_worker_message_body(websocket_parser * parser, const char * at, size_t length) {
    return 0;
}

int ion_process_worker_message_end(websocket_parser * parser) {
    return 0;
}


void _ion_worker_message_incoming(ion_buffer * bev, void * ctx) {
    ION_CB_BEGIN();
//    ion_process_worker * worker = (ion_process_worker *) ctx;



    ION_CB_END();
}

void _ion_worker_link_notify(ion_buffer * bev, short what, void * ctx) {
    ION_CB_BEGIN();
    ion_process_worker * worker = (ion_process_worker *) ctx;
    if(what & BEV_EVENT_EOF) {
        usleep(100000);
        int status = 0;
        pid_t p = waitpid(worker->pid, &status, WNOHANG | WUNTRACED);
        if(p == worker->pid) {
            if(WIFSIGNALED(status)) {
                worker->signal = WTERMSIG(status);
                worker->exit_status = status;
                worker->flags |= ION_WORKER_SIGNALED;
            } else if(WIFSTOPPED(status)) {
//                worker->signal = WSTOPSIG(status);
            } else {
                worker->exit_status = WEXITSTATUS(status);
                if(worker->exit_status) {
                    worker->flags |= ION_WORKER_FAILED;
                } else {
                    worker->flags |= ION_WORKER_DONE;
                }
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
    } else if(what & BEV_EVENT_ERROR) {

    } else if(what & BEV_EVENT_TIMEOUT) {

    } else if(what & BEV_EVENT_CONNECTED) {

    }
    ION_CB_END();
}

void ion_process_worker_spawn(ion_process_worker * worker) {
    int pid = 0;
    ion_buffer * master_buffer;
    ion_buffer * worker_buffer;

    if(ion_buffer_pair(&master_buffer, &worker_buffer) == false) {
        return;
    }
    bufferevent_setcb(master_buffer, _ion_worker_message_incoming, NULL, _ion_worker_link_notify, worker);
    bufferevent_setcb(worker_buffer, _ion_worker_message_incoming, NULL, _ion_worker_link_notify, worker);
    worker->parser = ecalloc(1, sizeof(websocket_parser));
    websocket_parser_init(worker->parser);
    worker->parser->data = worker;
    errno = 0;
    pid = fork();
    if(pid == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to spawn worker: %s", strerror(errno));
        return;
    } else if(pid) { // parent
        worker->flags |= ION_WORKER_CHILD | ION_WORKER_STARTED;
        worker->buffer = master_buffer;
        bufferevent_disable(worker_buffer, EV_READ | EV_WRITE);
        bufferevent_free(worker_buffer);
        worker->pid = pid;
        ion_process_add_child(pid, ION_PROC_CHILD_WORKER, ION_OBJ(worker));
    } else { // child
        if(event_reinit(GION(base)) == FAILURE) {
            zend_error(E_NOTICE, "Some events could not be restarted");
        }
        worker->flags |= ION_WORKER_MASTER | ION_WORKER_STARTED;
        worker->buffer = worker_buffer;
        worker->pid    = getppid();
        PHPDBG("0");

        if(worker->on_message) {
            zend_object_release(worker->on_message);
            worker->on_message = NULL;
        }
        if(worker->on_exit) {
            zend_object_release(worker->on_exit);
            worker->on_exit = NULL;
        }
        if(worker->on_connect) {
            zend_object_release(worker->on_connect);
            worker->on_connect = NULL;
        }
        if(worker->on_disconnect) {
            zend_object_release(worker->on_disconnect);
            worker->on_disconnect = NULL;
        }

//        zend_hash_clean(GION(workers));

        bufferevent_disable(master_buffer, EV_READ | EV_WRITE);
        bufferevent_free(master_buffer);
        zend_object_addref(ION_OBJ(worker));

        if(GION(master)) {
            zend_object_release(GION(master));
        }
        GION(master) = ION_OBJ(worker);

        if(worker->cb) {
            PHPDBG("1");
            zval container;
            ZVAL_OBJ(&container, ION_OBJ(worker));
            zval_add_ref(&container);
            PHPDBG("2");

//            pion_cb_void_with_1_arg(worker->cb, &container);
            zval_ptr_dtor(&container);
//            pion_cb_release(worker->cb);
            worker->cb = NULL;
            PHPDBG("3");

        }
    }
}

/** public function ION\Process\Worker::getStartedTime() : float */
CLASS_METHOD(ION_Process_Worker, getStartedTime) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    double time = worker->started_time.tv_sec;
    time += worker->started_time.tv_usec / 1e6;

    RETURN_DOUBLE(time);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, getStartedTime);

/** public function ION\Process\Worker::getPID() : int */
CLASS_METHOD(ION_Process_Worker, getPID) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    RETURN_LONG(worker->pid);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, getPID);

/** public function ION\Process\Worker::isAlive() : bool */
CLASS_METHOD(ION_Process_Worker, isAlive) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    if((worker->flags & ION_WORKER_FINISHED) || !(worker->flags & ION_WORKER_STARTED)) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, isAlive);

/** public function ION\Process\Worker::isExited() : bool */
CLASS_METHOD(ION_Process_Worker, isExited) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    if(worker->flags & ION_WORKER_FINISHED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, isExited);

/** public function ION\Process\Worker::isParent() : bool */
CLASS_METHOD(ION_Process_Worker, isParent) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    if(worker->flags & ION_WORKER_MASTER) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, isParent);


/** public function ION\Process\Worker::isChild() : bool */
CLASS_METHOD(ION_Process_Worker, isChild) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    if(worker->flags & ION_WORKER_MASTER) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, isChild);

/** public function ION\Process\Worker::isSignaled() : bool */
CLASS_METHOD(ION_Process_Worker, isSignaled) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    if(worker->flags & ION_WORKER_SIGNALED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, isSignaled);

/** public function ION\Process\Worker::isStarted() : int */
CLASS_METHOD(ION_Process_Worker, isStarted) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    if(worker->flags & ION_WORKER_STARTED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, isStarted);

/** public function ION\Process\Worker::getSignal() : int */
CLASS_METHOD(ION_Process_Worker, getSignal) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    RETURN_LONG(worker->signal);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, getSignal);

/** public function ION\Process\Worker::getExitStatus() : int */
CLASS_METHOD(ION_Process_Worker, getExitStatus) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    RETURN_LONG(worker->exit_status);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, getExitStatus);

/** public function ION\Process\Worker::onMessage() : ION\Sequence */
CLASS_METHOD(ION_Process_Worker, onMessage) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    if(!worker->on_message) {
        worker->on_message = ion_promisor_sequence_new(NULL);
    }
    zend_object_addref(worker->on_message);
    RETURN_OBJ(worker->on_message);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, onMessage);

/** public function ION\Process\Worker::onConnect() : ION\Promise */
CLASS_METHOD(ION_Process_Worker, onConnect) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    if(!worker->on_connect) {
        worker->on_connect = ION_OBJ(ion_promisor_promise_ex(0));
    }
    zend_object_addref(worker->on_connect);
    RETURN_OBJ(worker->on_connect);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, onConnect);

/** public function ION\Process\Worker::onDisconnect() : ION\Promise */
CLASS_METHOD(ION_Process_Worker, onDisconnect) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    if(!worker->on_disconnect) {
        worker->on_disconnect = ION_OBJ(ion_promisor_promise_ex(0));
    }
    zend_object_addref(worker->on_disconnect);
    RETURN_OBJ(worker->on_disconnect);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, onDisconnect);

/** public function ION\Process\Worker::onExit() : ION\Sequence */
CLASS_METHOD(ION_Process_Worker, onExit) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);

    if(!worker->on_exit) {
        worker->on_exit = ion_promisor_sequence_new(NULL);
    }
    zend_object_addref(worker->on_exit);
    RETURN_OBJ(worker->on_exit);
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, onExit);

/** public function ION\Process\Worker::event(string $name) : ION\Process\Event */
CLASS_METHOD(ION_Process_Worker, event) {
//    ion_process_worker * worker = get_this_instance(ion_process_worker);


}

METHOD_ARGS_BEGIN(ION_Process_Worker, event, 1)
    METHOD_ARG_STRING(name, 0)
METHOD_ARGS_END();

/** public function ION\Process\Worker::run(string $name) : self */
CLASS_METHOD(ION_Process_Worker, run) {
    ion_process_worker * worker = get_this_instance(ion_process_worker);
    zval * zcb = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(zcb)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(worker->cb) {
        pion_cb_release(worker->cb);
    }
    worker->cb = pion_cb_create_from_zval(zcb);

    ion_deferred_queue_push(worker_spawn_method, Z_OBJ_P(getThis()));

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Process_Worker, run, 1)
    METHOD_ARG_CALLBACK(name, 0, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_Process_Worker, _spawn) {
    ion_process_worker_spawn(get_this_instance(ion_process_worker));
}

METHOD_WITHOUT_ARGS(ION_Process_Worker, _spawn);

CLASS_METHODS_START(ION_Process_Worker)
    METHOD(ION_Process_Worker, getStartedTime, ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, getPID,         ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, isAlive,        ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, isStarted,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, isExited,       ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, isParent,       ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, isChild,        ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, isSignaled,     ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, getSignal,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, getExitStatus,  ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, onMessage,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, onConnect,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, onDisconnect,   ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, onMessage,      ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, onExit,         ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, event,          ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, run,            ZEND_ACC_PUBLIC)
    METHOD(ION_Process_Worker, _spawn,         ZEND_ACC_PRIVATE)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Process_Worker) {
    pion_register_class(ION_Process_Worker, "ION\\Process\\Worker", ion_process_worker_init, CLASS_METHODS(ION_Process_Worker));
    pion_init_std_object_handlers(ION_Process_Worker);
    pion_set_object_handler(ION_Process_Worker, free_obj, ion_process_worker_free);
    pion_set_object_handler(ION_Process_Worker, clone_obj, NULL);

    websocket_parser_settings_init(&parser_settings);
    parser_settings.on_frame_header = ion_process_worker_message_begin;
    parser_settings.on_frame_body = ion_process_worker_message_body;
    parser_settings.on_frame_end = ion_process_worker_message_end;

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process_Worker) {
    ALLOC_HASHTABLE(GION(workers));
    zend_hash_init(GION(workers), 128, NULL, zval_ptr_dtor_wrapper, 0);
    worker_spawn_method = pion_cb_fetch_method("ION\\Process\\Worker", "_spawn");
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process_Worker) {
    zend_hash_clean(GION(workers));
    zend_hash_destroy(GION(workers));
    FREE_HASHTABLE(GION(workers));
    pion_cb_release(worker_spawn_method);
    if(GION(master)) {
        zend_object_release(GION(master));
    }
    return SUCCESS;
}