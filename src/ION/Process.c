#include "Process.h"
#include <sys/resource.h>
#ifdef HAVE_KILL
#include <signal.h>
#endif
#include <event2/event.h>

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

struct passwd * get_pw_by_zval(zval * user) {
    struct passwd * pw;
    errno = 0;
    if(Z_TYPE_P(user) == IS_STRING) {
        if (NULL == (pw = getpwnam(Z_STRVAL_P(user)))) {
            if(errno) {
                ThrowRuntimeEx(errno, "Failed to get info by user name %s: %s", Z_STRVAL_P(user), strerror(errno));
            } else {
                ThrowInvalidArgumentEx("User %s not found", Z_STRVAL_P(user));
            }
            return NULL;
        }
    } else if(Z_TYPE_P(user) == IS_LONG) {
        if (NULL == (pw = getpwuid((uid_t)Z_LVAL_P(user)))) {
            if(errno) {
                ThrowRuntimeEx(errno, "Failed to get info by UID %d: %s", Z_LVAL_P(user), strerror(errno));
            } else {
                ThrowInvalidArgumentEx("UID %d not found", Z_LVAL_P(user));
            }
            return NULL;
        }
    } else {
        ThrowInvalidArgument("Invalid user identifier");
        return NULL;
    }
    return pw;
}

/** public function ION\Process::getUser($user = null) : array */
CLASS_METHOD(ION_Process, getUser) {
    struct passwd * pw;
    zval * zuser = NULL;

    PARSE_ARGS("|z", &zuser);

    if(zuser == NULL || Z_TYPE_P(zuser) == IS_NULL) {
        if(!zuser) {
            ALLOC_INIT_ZVAL(zuser);
        }
        ZVAL_LONG(zuser, getuid());
    }

    pw = get_pw_by_zval(zuser);
    zval_ptr_dtor(&zuser);

    array_init(return_value);

    add_assoc_string(return_value, "name",   pw->pw_name, 1);
    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "home",   pw->pw_dir, 1);
    add_assoc_string(return_value, "shell",  pw->pw_shell, 1);
}

METHOD_ARGS_BEGIN(ION_Process, getUser, 0)
    METHOD_ARG(user, 0)
METHOD_ARGS_END()


/** public function ION\Process::setUser($user, $set_group = true) : array */
CLASS_METHOD(ION_Process, setUser) {
    struct passwd *pw;
    zval *zuser = NULL;
    zend_bool set_group = 1;

    PARSE_ARGS("z|b", &zuser, &set_group);

    pw = get_pw_by_zval(zuser);

    array_init(return_value);

    add_assoc_string(return_value, "name",   pw->pw_name, 1);
    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "home",   pw->pw_dir, 1);
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


CLASS_METHODS_START(ION_Process)
    METHOD(ION_Process, fork,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, signal, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getParentPid, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getUser, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setUser, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPriority, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setPriority, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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