#include "Process.h"
#include <sys/resource.h>
#ifdef HAVE_KILL
#include <signal.h>
#endif
#include <event.h>

DEFINE_CLASS(ION_Process);

/** public function ION\Process::fork(int $flags = 0, Stream &$ipc = null) : int */
CLASS_METHOD(ION_Process, fork) {
#ifdef HAVE_FORK
    int pid = 0;
    long flags = 0;
    zval *ipc = NULL;

    PARSE_ARGS("|lz", &flags, &ipc);
    errno = 0;
    pid = fork();
    if(pid == -1) {
        ThrowRuntimeEx(errno, "Fork failed: %s", strerror(errno));
    } else if(pid) { // parent
        RETURN_LONG(pid);
    } else { // child
        if(event_reinit(ION(base)) == FAILURE) {
            php_error(E_NOTICE, "Some events could not be re-added");
        }
        RETURN_LONG(0);
    }
#else
    ThrowUnsupported("The platform does not supported process' forks");
#endif
}

METHOD_ARGS_BEGIN(ION_Process, fork, 0)
    METHOD_ARG(flags, 0)
    METHOD_ARG(ipc, 1)
METHOD_ARGS_END()


/** public function ION\Process::signal($signo, $pid, $to_group = false) : bool */
CLASS_METHOD(ION_Process, signal) {
#ifdef HAVE_KILL
    long signal, pid;
    zend_bool to_group = 0;

    PARSE_ARGS("ll|b", &signal, &pid, &to_group);
    if(to_group) {
        RETURN_LONG(killpg((pid_t)pid, (int)signal) < 0);
    } else {
        RETURN_LONG(kill((pid_t)pid, (int)signal) < 0);
    }
#else
    ThrowUnsupported("ION\\Process::signal() not supported by platform");
#endif
}

METHOD_ARGS_BEGIN(ION_Process, signal, 2)
    METHOD_ARG(signo, 0)
    METHOD_ARG(pid, 0)
    METHOD_ARG(to_group, 0)
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



struct passwd * get_pw_by_zval(zval * zuser TSRMLS_DC) {
    struct passwd * pw;
    errno = 0;
    if(Z_TYPE_P(zuser) == IS_STRING) {
        pw = getpwnam(Z_STRVAL_P(zuser));
    } else if(Z_TYPE_P(zuser) == IS_LONG) {
        pw = getpwuid((uid_t)Z_LVAL_P(zuser));
    } else {
        ThrowInvalidArgument("Invalid user identifier");
        return NULL;
    }
    if (NULL == pw) {
        if(errno) {
            if(Z_TYPE_P(zuser) == IS_STRING) {
                ThrowRuntimeEx(errno, "Failed to get info by user name %s: %s", Z_STRVAL_P(zuser), strerror(errno));
            } else {
                ThrowRuntimeEx(errno, "Failed to get info by UID %d: %s", Z_LVAL_P(zuser), strerror(errno));
            }
        }
        return NULL;
    }
    return pw;
}

/** public function ION\Process::getUser($user = null) : array|bool */
CLASS_METHOD(ION_Process, getUser) {
    struct passwd * pw;
    zval * zuser = NULL;
    zend_bool me = 0;

    PARSE_ARGS("|z", &zuser);

    if(zuser == NULL || Z_TYPE_P(zuser) == IS_NULL) {
        if(!zuser) {
            ALLOC_INIT_ZVAL(zuser);
        }
        ZVAL_LONG(zuser, getuid());
        me = 1;
    }

    pw = get_pw_by_zval(zuser TSRMLS_CC);
    if(me) {
        zval_ptr_dtor(&zuser);
    }

    array_init(return_value);

    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_string(return_value, "name",   pw->pw_name,  1);
    add_assoc_string(return_value, "gecos",  pw->pw_gecos, 1);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "home",   pw->pw_dir,   1);
    add_assoc_string(return_value, "shell",  pw->pw_shell, 1);
}

METHOD_ARGS_BEGIN(ION_Process, getUser, 0)
    METHOD_ARG(user, 0)
METHOD_ARGS_END()


/** public function ION\Process::setUser($user, $set_group = true) : array */
CLASS_METHOD(ION_Process, setUser) {
    struct passwd * pw;
    zval * zuser = NULL;
    zend_bool set_group = 1;

    PARSE_ARGS("z|b", &zuser, &set_group);

    pw = get_pw_by_zval(zuser TSRMLS_CC);

    array_init(return_value);

    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_string(return_value, "name",   pw->pw_name,  1);
    add_assoc_string(return_value, "gecos",  pw->pw_gecos, 1);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "home",   pw->pw_dir,   1);
    add_assoc_string(return_value, "shell",  pw->pw_shell, 1);

    if(set_group && setgid(pw->pw_gid)) {
        ThrowRuntimeEx(errno, "Failed to set GID %d: %s", (int)pw->pw_gid, strerror(errno));
        return;
    }

    if(setuid(pw->pw_uid)) {
        ThrowRuntimeEx(errno, "Failed to set UID %d: %s", (int)pw->pw_uid, strerror(errno));
        return;
    }
}

METHOD_ARGS_BEGIN(ION_Process, setUser, 1)
    METHOD_ARG(user, 0)
    METHOD_ARG(set_group, 0)
METHOD_ARGS_END()

/** public function ION\Process::getPriority($pid = null) : int */
CLASS_METHOD(ION_Process, getPriority) {
    long pid = getpid();

    PARSE_ARGS("|l", &pid);

    errno = 0;

    int pri = getpriority(PRIO_PROCESS, (id_t)pid);

    if (errno) {
        ThrowRuntimeEx(errno, "Failed to get process %d priority: %s", pid, strerror(errno));
        return;
    }

    RETURN_LONG(pri);
}

METHOD_ARGS_BEGIN(ION_Process, getPriority, 0)
    METHOD_ARG(pid, 0)
METHOD_ARGS_END()

/** public function ION\Process::setPriority($priority, $pid = null) : int */
CLASS_METHOD(ION_Process, setPriority) {
    long pid = getpid();
    long priority = 0;

    PARSE_ARGS("l|l", &priority, &pid);

    int prev_priority = getpriority(PRIO_PROCESS, (id_t)pid);

    if(priority < -20 || priority > 20) {
        ThrowInvalidArgument("Invalid priority value");
        return;
    }

    errno = 0;
    if (setpriority(PRIO_PROCESS, (id_t)pid, (int)priority)) {
        ThrowRuntimeEx(errno, "Failed to set process %d priority %d: %s", (int)pid, (int)priority, strerror(errno));
        return;
    }

    RETURN_LONG(prev_priority);
}

METHOD_ARGS_BEGIN(ION_Process, setPriority, 1)
    METHOD_ARG(priority, 0)
    METHOD_ARG(pid, 0)
METHOD_ARGS_END()

typedef struct bufferevent bevent;

typedef struct _ion_exec {
    int      pid;
    zval     *defer;
    bevent   *out;
    bevent   *err;
#ifdef ZTS
    void ***thread_ctx;
#endif
} IONExec;

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
//            deferredResolve(exec->defer, result);
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

/** public function ION\Process::exec($priority, $pid = null) : int */
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
//        deferredStore(exec->defer, exec, NULL);
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
    METHOD(ION_Process, fork,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, signal, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getParentPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getUser, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setUser, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPriority, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setPriority, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
//    METHOD(ION_Process, exec, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Process) {
    PION_REGISTER_PLAIN_CLASS(ION_Process, "ION\\Process");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Process) {
    return SUCCESS;
}