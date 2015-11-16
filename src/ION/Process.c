#include "Process.h"
#include <sys/resource.h>
#include <signal.h>
#include <event.h>


zend_class_entry * ion_ce_ION_Process;
zend_object_handlers ion_oh_ION_Process;
zend_class_entry * ion_ce_ION_Process_ExecResult;
zend_object_handlers ion_oh_ION_Process_ExecResult;

/** public function ION\Process::fork(int $flags = 0, Stream &$ipc = null) : int */
CLASS_METHOD(ION_Process, fork) {
    int pid = 0;
    zend_long flags = 0;

    ZEND_PARSE_PARAMETERS_START(0,1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    errno = 0;
    pid = fork();
    if(pid == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to spawn process: %s", strerror(errno));
        return;
    } else if(pid) { // parent
        RETURN_LONG(pid);
    } else { // child
        if(!ion_reinit()) {
            php_error(E_NOTICE, "Some events could not be re-added");
        }
        RETURN_LONG(0);
    }
}

METHOD_ARGS_BEGIN(ION_Process, fork, 0)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END()


/** public function ION\Process::kill(int $signo, int $pid, bool $to_group = false) : bool */
CLASS_METHOD(ION_Process, kill) {
    zend_long signal = 0;
    zend_long pid = 0;
    zend_bool to_group = 0;
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(signal)
        Z_PARAM_LONG(pid)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(to_group)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(to_group) {
        RETURN_BOOL(killpg((pid_t)pid, (int)signal) < 0);
    } else {
        RETURN_BOOL(kill((pid_t)pid, (int)signal) < 0);
    }
}

METHOD_ARGS_BEGIN(ION_Process, kill, 2)
    METHOD_ARG_LONG(signo, 0)
    METHOD_ARG_LONG(pid, 0)
    METHOD_ARG_BOOL(to_group, 0)
METHOD_ARGS_END()

void ion_process_signal_dtor(zend_object * sequence) {
    ion_promisor * promisor = get_object_instance(sequence, ion_promisor);
    if(promisor->object) {
        ion_event * event = promisor->object;
        event_del(event);
        event_free(event);
        promisor->object = NULL;
    }
}


void ion_process_clean_signal(zval * zs) {
    zend_object  * sequence = Z_PTR_P(zs);
    ion_process_signal_dtor(sequence);
    zend_object_release(sequence);
}


void ion_process_signal(int signo, short flags, void * ctx) {
    ION_LOOP_CB_BEGIN();
    zend_object * sequence = (zend_object *) ctx;
    zval          signal;

    ZVAL_LONG(&signal, signo);
    ion_promisor_sequence_invoke(sequence, &signal);

    ION_LOOP_CB_END();
}

/** public function ION\Process::signal(int $signo) : Sequence */
CLASS_METHOD(ION_Process, signal) {
    zend_long signo = 0;
    zend_object * sequence = NULL;
    ion_event * event;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_LONG(signo)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    sequence = zend_hash_index_find_ptr(ION(signals), (zend_ulong)signo);
    if(sequence) {
        obj_add_ref(sequence);
        RETURN_OBJ(sequence);
    }
    sequence = ion_promisor_sequence_new(NULL);
    event = evsignal_new(ION(base), (int)signo, ion_process_signal, sequence);
    if(evsignal_add(event, NULL) == FAILURE) {
        zend_object_release(sequence);
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to listening signal %d", signo);
        return;
    }
    ion_promisor_store(sequence, event);
    ion_promisor_dtor(sequence, ion_process_signal_dtor);
    zval * pz;
    pz = zend_hash_index_add_ptr(ION(signals), (zend_ulong)signo, (void *)sequence);
    if(!pz) {
        zend_object_release(sequence);
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to store signal (%d) handler", signo);
        return;
    }

    obj_add_ref(sequence);
    RETURN_OBJ(sequence);
}

METHOD_ARGS_BEGIN(ION_Process, signal, 1)
    METHOD_ARG_LONG(signo, 0)
METHOD_ARGS_END()

/** public function ION\Process::clearSignal(int $signo = -1) : void */
CLASS_METHOD(ION_Process, clearSignal) {
    zend_long signo = -1;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_LONG(signo)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(signo >= 0) {
        zend_hash_index_del(ION(signals), (zend_ulong)signo);
    } else {
        zend_hash_clean(ION(signals));
    }
}

METHOD_ARGS_BEGIN(ION_Process, clearSignal, 0)
    METHOD_ARG_LONG(signo, 0)
METHOD_ARGS_END()

/** public function ION\Process::getPid() : int */
CLASS_METHOD(ION_Process, getPid) {
    RETURN_LONG(getpid());
}

METHOD_WITHOUT_ARGS(ION_Process, getPid);

/** public function ION\Process::getParentPid() : int */
CLASS_METHOD(ION_Process, getParentPid) {
    RETURN_LONG(getppid());
}

METHOD_WITHOUT_ARGS(ION_Process, getParentPid);



struct passwd * get_pw_by_zval(zval * zuser) {
    struct passwd * pw;
    errno = 0;
    if(Z_TYPE_P(zuser) == IS_STRING) {
        pw = getpwnam(Z_STRVAL_P(zuser));
    } else if(Z_TYPE_P(zuser) == IS_LONG) {
        pw = getpwuid((uid_t)Z_LVAL_P(zuser));
    } else {
        zend_throw_exception(ion_class_entry(InvalidArgumentException), "Invalid user identifier", 0);
        return NULL;
    }
    if (NULL == pw) {
        if(errno) {
            if(Z_TYPE_P(zuser) == IS_STRING) {
                zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to get info by user name %s: %s", Z_STRVAL_P(zuser), strerror(errno));
            } else {
                zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to get info by UID %d: %s", Z_LVAL_P(zuser), strerror(errno));
            }
        }
        return NULL;
    }
    return pw;
}

/** public function ION\Process::getUser($user = null) : array|bool */
CLASS_METHOD(ION_Process, getUser) {
    struct passwd * pw;
    zval   * user = NULL;
    zval     me;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(user, 1 , 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(user == NULL) {
        ZVAL_LONG(&me, getuid());
        user = &me;
    }

    pw = get_pw_by_zval(user);

    array_init(return_value);

    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_string(return_value, "name",   pw->pw_name);
    add_assoc_string(return_value, "gecos",  pw->pw_gecos);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "dir",    pw->pw_dir);
    add_assoc_string(return_value, "shell",  pw->pw_shell);
}

METHOD_ARGS_BEGIN(ION_Process, getUser, 0)
    METHOD_ARG(user, 0)
METHOD_ARGS_END()


/** public function ION\Process::setUser(string|int $user, bool $set_group = true) : array */
CLASS_METHOD(ION_Process, setUser) {
    struct passwd * pw;
    zval * user = NULL;
    zend_bool set_group = 1;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_ZVAL(user)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(set_group)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    pw = get_pw_by_zval(user);

    array_init(return_value);

    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_string(return_value, "name",   pw->pw_name);
    add_assoc_string(return_value, "gecos",  pw->pw_gecos);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "dir",    pw->pw_dir);
    add_assoc_string(return_value, "shell",  pw->pw_shell);

    if(set_group && setgid(pw->pw_gid)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to set GID %d: %s", (int)pw->pw_gid, strerror(errno));
        return;
    }

    if(setuid(pw->pw_uid)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to set UID %d: %s", (int)pw->pw_gid, strerror(errno));
        return;
    }
}

METHOD_ARGS_BEGIN(ION_Process, setUser, 1)
    METHOD_ARG(user, 0)
    METHOD_ARG_BOOL(set_group, 0)
METHOD_ARGS_END()

/** public function ION\Process::getPriority($pid = null) : int */
CLASS_METHOD(ION_Process, getPriority) {
    zend_long pid = getpid();

    ZEND_PARSE_PARAMETERS_START(0,1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(pid)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    errno = 0;

    int pri = getpriority(PRIO_PROCESS, (id_t)pid);

    if (errno) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to get process %d priority: %s", pid, strerror(errno));
        return;
    }

    RETURN_LONG(pri);
}

METHOD_ARGS_BEGIN(ION_Process, getPriority, 0)
    METHOD_ARG_LONG(pid, 0)
METHOD_ARGS_END()

/** public function ION\Process::setPriority($priority, $pid = null) : int */
CLASS_METHOD(ION_Process, setPriority) {
    zend_long pid = getpid();
    zend_long priority = 0;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_LONG(priority)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(pid)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    int prev_priority = getpriority(PRIO_PROCESS, (id_t)pid);

    if(priority < -20 || priority > 20) {
        zend_throw_exception(ion_class_entry(InvalidArgumentException), "InvInvalid priority value", 0);
        return;
    }

    errno = 0;
    if (setpriority(PRIO_PROCESS, (id_t)pid, (int)priority)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to set process %d priority %d: %s", (int)pid, (int)priority, strerror(errno));
        return;
    }

    RETURN_LONG(prev_priority);
}

METHOD_ARGS_BEGIN(ION_Process, setPriority, 1)
    METHOD_ARG_LONG(priority, 0)
    METHOD_ARG_LONG(pid, 0)
METHOD_ARGS_END()

//inline void _ion_exec_callback(bevent *bev, short what, void *arg) {
//    char *output;
//    int status = 0, pid = 0;
//    long size = 0;
//    IONExec *exec = (IONExec *)arg;
//
//    if(what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
//
//        pid = waitpid(exec->pid, &status, WNOHANG);
//        if(exec->pid == pid) {
//            zval *result = NULL;
//            ALLOC_INIT_ZVAL(result);
//            array_init(result);
//            add_assoc_long(result, "pid", pid);
//            add_assoc_long(result, "stauts", status);
//            if(status < 256) {
//                add_assoc_long(result, "killed", status);
//            } else {
//                add_assoc_long(result, "killed", 0);
//            }
//            size = evbuffer_get_length(bufferevent_get_input(exec->out));
//            if(size) {
//                output = emalloc(size);
//                bufferevent_read(exec->out, (void *)output, size);
//                add_assoc_stringl(result, "stdout", output, size, 0);
//            } else {
//                add_assoc_stringl(result, "stdout", "", 0, 1);
//            }
//            size = evbuffer_get_length(bufferevent_get_input(exec->err));
//            if(size) {
//                output = emalloc(size);
//                bufferevent_read(exec->err, (void *)output, size);
//                add_assoc_stringl(result, "stderr", output, size, 0);
//            } else {
//                add_assoc_stringl(result, "stderr", "", 0, 1);
//            }
//            ion_deferred_done(exec->defer, result);
//            zval_ptr_dtor(&result);
//        } else {
//            // @todo call ion_defer_fail()
//            php_error(E_WARNING, "Exec corrupt (pid %d)", exec->pid);
//        }
//        bufferevent_disable(exec->out, EV_READ | EV_WRITE);
//        bufferevent_free(exec->out);
//        bufferevent_disable(exec->err, EV_READ | EV_WRITE);
//        bufferevent_free(exec->err);
//        efree(exec);
//    }
//}

//inline void _ion_exec_cancel(zval *error, void * deferred TSRMLS_DC) {
//    IONExec *exec = (IONExec *)arg;
//    bufferevent_disable(exec->out, EV_READ | EV_WRITE);
//    bufferevent_free(exec->out);
//    bufferevent_disable(exec->err, EV_READ | EV_WRITE);
//    bufferevent_free(exec->err);
//    efree(exec);
//}

zend_string * ion_buffer_read_all(ion_buffer * buffer) {
    size_t incoming_length = evbuffer_get_length(bufferevent_get_input(buffer));
    zend_string * data;

    if(!incoming_length) {
        return zend_string_init("", 0, 0);
    }

    data = zend_string_alloc(incoming_length, 0);
    ZSTR_LEN(data) = bufferevent_read(buffer, ZSTR_VAL(data), incoming_length);
    if (ZSTR_LEN(data) > 0) {
        ZSTR_VAL(data)[ZSTR_LEN(data)] = '\0';
        return data;
    } else {
        zend_string_free(data);
        return NULL;
    }
}

void ion_exec_callback(bevent * bev, short what, void * arg) {
    ion_exec * exec = (ion_exec *) arg;
    int        pid;
    int        status;
    zval       result;
    zend_string * out;
    zend_string * err;

    if(what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        pid = waitpid(exec->pid, &status, WNOHANG);
        if(exec->pid != pid) {
            // to do something. who get my pid?
        }
        object_init_ex(&result, ion_class_entry(ION_Process_ExecResult));
        pion_update_property_long(ION_Process_ExecResult, &result, "pid", exec->pid);
        pion_update_property_long(ION_Process_ExecResult, &result, "status", status);
        pion_update_property_str(ION_Process_ExecResult, &result, "command", exec->command);
        out = ion_buffer_read_all(exec->out);
        if(out) {
            pion_update_property_str(ION_Process_ExecResult, &result, "stdout", out);
        }
        err = ion_buffer_read_all(exec->err);
        if(err) {
            pion_update_property_str(ION_Process_ExecResult, &result, "stderr", err);
        }
        ion_promisor_done(exec->deferred, &result);
        zend_object_release(exec->deferred);
        bufferevent_disable(exec->out, EV_READ | EV_WRITE);
        bufferevent_free(exec->out);
        close(exec->stdout_fd);
        bufferevent_disable(exec->err, EV_READ | EV_WRITE);
        bufferevent_free(exec->err);
        close(exec->stderr_fd);
        efree(exec);
    }
}

/** public function ION\Process::exec($priority, $pid = null) : int */
CLASS_METHOD(ION_Process, exec) {
    zend_string * command = NULL;
    zval        * options = NULL;
    int           pid;
    int           out_pipes[2];
    int           err_pipes[2];
    ion_exec    * exec;
//    char       ** env;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_STR(command)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_EX(options, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(pipe( out_pipes )) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to initializate stdout stream: %s", strerror(errno));
        return;
    }
    if(pipe( err_pipes )) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to initializate stdin stream: %s", strerror(errno));
        return;
    }

    pid = fork();
    if(pid == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, "Failed to spawn process for command: %s", strerror(errno));
        return;
    } else if(pid) { // parent
        close( out_pipes[1] );
        close( err_pipes[1] );
        exec = ecalloc(1, sizeof(ion_exec));
        exec->command   = zend_string_copy(command);
        exec->stdout_fd = out_pipes[0];
        exec->stderr_fd = err_pipes[0];
        exec->err    = bufferevent_new(exec->stderr_fd, NULL, NULL, NULL, (void *)exec);
        exec->out    = bufferevent_new(exec->stdout_fd, NULL, NULL, ion_exec_callback, (void *)exec);
        if(bufferevent_base_set(ION(base), exec->err) == FAILURE) {
            bufferevent_free(exec->err);
            close(exec->stdout_fd);
            close(exec->stderr_fd);
            efree(exec);
            zend_throw_exception(ion_class_entry(ION_RuntimeException),"Failed to initializate spawned process", 0);
            return;
        }
        if(bufferevent_base_set(ION(base), exec->out) == FAILURE) {
            bufferevent_free(exec->err);
            bufferevent_free(exec->out);
            close(exec->stdout_fd);
            close(exec->stderr_fd);
            efree(exec);
            zend_throw_exception(ion_class_entry(ION_RuntimeException),"Failed to initializate spawned process", 0);
            return;
        }
        exec->deferred = ion_promisor_deferred_new_ex(NULL);
        ion_promisor_store(exec->deferred, exec);
        obj_add_ref(exec->deferred);
        RETURN_OBJ(exec->deferred);
    } else {  // child
        if (dup2( out_pipes[1], 1 ) < 0 ) {
            perror(strerror(errno));
            _exit(127);
        }
        if (dup2( err_pipes[1], 2 ) < 0 ) {
            perror(strerror(errno));
            _exit(127);
        }
        if(execl("/bin/sh", "sh", "-c", command->val, NULL)) {
            perror(strerror(errno));
        }
        _exit(127);
    }
}


METHOD_ARGS_BEGIN(ION_Process, exec, 1)
    METHOD_ARG(command, 0)
    METHOD_ARG_ARRAY(options, 0, 0)
METHOD_ARGS_END()

//CLASS_METHOD(ION_Process, exec) {
//#ifdef HAVE_FORK
//    int out_pipes[2], err_pipes[2];
//    char *command;
//    int pid = 0;
//    long command_len = 0;
//    IONExec *exec;
//    HashTable *options;
//
//    PARSE_ARGS("s|h", &command, &command_len, &options);
//
//    errno = 0;
//    if(pipe( out_pipes )) {
//        ThrowRuntimeEx(errno, "Execute command failed (stdout pipe): %s", strerror(errno));
//        return;
//    }
//
//    if(pipe( err_pipes )) {
//        ThrowRuntimeEx(errno, "Execute command failed (stdout pipe): %s", strerror(errno));
//        return;
//    }
//
//    pid = fork();
//    if(pid == -1) {
//        ThrowRuntimeEx(errno, "Execute command failed (fork): %s", strerror(errno));
//        return;
//    } else if(pid) { // parent
//        if(close( out_pipes[1] )) {
//            kill(pid, SIGKILL);
//            ThrowRuntimeEx(errno, "Execute command failed (close stdout pipe#1): %s", strerror(errno));
//            return;
//        }
//        if(close( err_pipes[1] )) {
//            kill(pid, SIGKILL);
//            ThrowRuntimeEx(errno, "Execute command failed (close stderr pipe#1): %s", strerror(errno));
//            return;
//        }
//        exec = emalloc(sizeof(IONExec));
//        memset(exec, 0, sizeof(IONExec));
//        exec->pid = pid;
//
//        // create stdout pipe
//        exec->out = bufferevent_new(out_pipes[0], NULL, NULL, (bufferevent_event_cb)_ion_exec_callback, (void *)exec);
//        if(bufferevent_base_set(ION(base), exec->out)) {
//            bufferevent_free(exec->out);
//            efree(exec);
//            ThrowRuntimeEx(1, "Execute command failed: stdout buffer event set failed");
//        }
//        if(bufferevent_enable(exec->out, EV_READ)) {
//            bufferevent_free(exec->out);
//            efree(exec);
//            ThrowRuntimeEx(1, "Execute command failed: stdout buffer event enable failed");
//        }
//        bufferevent_setwatermark(exec->out, EV_READ, 0x1000000, 0x1000000);
//
//        // create stderr pipe
//        exec->err = bufferevent_new(err_pipes[0], NULL, NULL, (bufferevent_event_cb)_ion_exec_callback, (void *)exec);
//        if(bufferevent_base_set(ION(base), exec->err)) {
//            bufferevent_free(exec->out);
//            bufferevent_free(exec->err);
//            efree(exec);
//            ThrowRuntimeEx(1, "Execute command failed: stderr buffer event set failed");
//        }
//        if(bufferevent_enable(exec->err, EV_READ)) {
//            bufferevent_free(exec->out);
//            bufferevent_free(exec->err);
//            efree(exec);
//            ThrowRuntimeEx(1, "Execute command failed: stderr buffer event enable failed");
//        }
//        bufferevent_setwatermark(exec->err, EV_READ, 0x1000000, 0x1000000);
//        exec->defer = deferredNewInternal(_ion_exec_cancel, 1);
//        ion_deferred_store(exec->defer, exec, NULL);
//        zval_add_ref(&exec->defer);
//        RETURN_ZVAL(exec->defer, 1, 0);
//    } else { // child
//        if (dup2( out_pipes[1], 1 ) < 0 ) {
//            perror(strerror(errno));
//            _exit(127);
//        }
//        if (dup2( err_pipes[1], 2 ) < 0 ) {
//            perror(strerror(errno));
//            _exit(127);
//        }
//        if(execl("/bin/sh", "sh", "-c", command, NULL)) {
//            perror(strerror(errno));
//        }
//        _exit(127);
//    }
//#else
//    ThrowUnsupported("ION\\Process::exec() not supported by this platform. Reason: no fork function");
//#endif
//}
//
//METHOD_ARGS_BEGIN(ION_Process, exec, 1)
//    METHOD_ARG(command, 0)
//    METHOD_ARG(options, 0)
//METHOD_ARGS_END()


CLASS_METHODS_START(ION_Process)
    METHOD(ION_Process, fork,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, kill,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, signal,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, clearSignal,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPid,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getParentPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getUser,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setUser,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPriority,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setPriority,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, exec,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

CLASS_METHODS_START(ION_Process_ExecResult)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Process) {
    PION_REGISTER_STATIC_CLASS(ION_Process, "ION\\Process");
    PION_REGISTER_DEFAULT_CLASS(ION_Process_ExecResult, "ION\\Process\\ExecResult");
    PION_CLASS_PROP_STRING(ION_Process_ExecResult, "command", "", ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_LONG(ION_Process_ExecResult, "pid", 0, ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_STRING(ION_Process_ExecResult, "stdout", "", ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_STRING(ION_Process_ExecResult, "stderr", "", ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_LONG(ION_Process_ExecResult, "status", -1, ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_BOOL(ION_Process_ExecResult, "signaled", 0, ZEND_ACC_PUBLIC);
    PION_CLASS_PROP_LONG(ION_Process_ExecResult, "signal", 0, ZEND_ACC_PUBLIC);
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process) {
    ALLOC_HASHTABLE(ION(signals));
    zend_hash_init(ION(signals), 128, NULL, ion_process_clean_signal, 0);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process) {
    zend_hash_clean(ION(signals));
    zend_hash_destroy(ION(signals));
    FREE_HASHTABLE(ION(signals));
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Process) {
    return SUCCESS;
}