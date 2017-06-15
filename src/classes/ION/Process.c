#include "ion.h"
#include "ext/standard/php_var.h"
#include <signal.h>
#include <grp.h>


zend_class_entry * ion_ce_ION_Process;
zend_object_handlers ion_oh_ION_Process;
zend_class_entry * ion_ce_ION_ProcessException;
zend_object_handlers ion_oh_ION_ProcessException;

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
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_SPAWN_FAIL, strerror(errno));
        return;
    } else if(pid) { // parent
        RETURN_LONG(pid);
    } else { // child
        if(event_reinit(GION(base)) == FAILURE) {
            php_error(E_NOTICE, ERR_ION_REINIT_FAILED);
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

void _ion_process_signal_free(ion_event * event) {
    ion_promisor  * sequence = event_get_callback_arg(event);
    ion_promisor_remove_object(sequence);
    ion_object_release(sequence);
    event_del(event);
    event_free(event);
}

void _ion_process_signal_dtor(zval * object) {
    _ion_process_signal_free(Z_PTR_P(object));
}


void _ion_process_signal(int signo, short flags, void * ctx) {
    ION_CB_BEGIN();

    ion_promisor_done_long((ion_promisor *) ctx, signo);

    ION_CB_END();
}

/** public function ION\Process::signal(int $signo) : Sequence */
CLASS_METHOD(ION_Process, signal) {
    zend_long      signo    = 0;
    ion_process_signal * signal = NULL;
//    ion_event    * event;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_LONG(signo)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    signal = zend_hash_index_find_ptr(GION(signals), (zend_ulong)signo);
    if(signal) {
        ion_object_addref(signal->sequence);
        RETURN_OBJ(ION_OBJECT_ZOBJ(signal->sequence));
    }
    signal = ecalloc(1, sizeof(ion_process_signal));
    signal->signo = signo;
    signal->sequence = ion_promisor_sequence_new(NULL);
//    ion_promisor_set_autoclean(sequence, ion_process_autoclean_signal);
    signal->event = evsignal_new(GION(base), (int)signo, _ion_process_signal, signal);
    if(evsignal_add(signal->event, NULL) == FAILURE) {
        ion_object_release(signal->sequence);
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, signo, ERR_ION_PROCESS_SIGNAL_EVENT_FAIL, signo);
        return;
    }
    ion_promisor_set_object_ptr(signal->sequence, signal->event, _ion_process_signal_dtor);
    zval * pz;
    pz = zend_hash_index_add_ptr(GION(signals), (zend_ulong)signo, (void *)signal);
    if(!pz) {
        ion_object_release(signal->sequence);
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, signo, ERR_ION_PROCESS_SIGNAL_STORE_FAIL, signo);
        return;
    }

    ion_object_addref(signal->sequence);
    RETURN_OBJ(ION_OBJECT_ZOBJ(signal->sequence));
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
        zend_hash_index_del(GION(signals), (zend_ulong)signo);
    } else {
        zend_hash_clean(GION(signals));
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


struct passwd * ion_get_pw_by_zval(zval * zuser) {
    errno = 0;
    // todo: add more ZTS there
    if(Z_TYPE_P(zuser) == IS_STRING) {
        return getpwnam(Z_STRVAL_P(zuser));
    } else if(Z_TYPE_P(zuser) == IS_LONG) {
        return getpwuid((uid_t)Z_LVAL_P(zuser));
    } else {
        return NULL;
    }
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

    pw = ion_get_pw_by_zval(user);

    if(!pw) {
        RETURN_FALSE;
    }

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


/** public function ION\Process::getGroup(mixed $group = null) : array|bool */
CLASS_METHOD(ION_Process, getGroup) {
    struct group * g;
    zval   * group = NULL;
    zval     members;
    zval     me;
    int      count;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(group, 1 , 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(group == NULL) {
        ZVAL_LONG(&me, getgid());
        group = &me;
    }

    errno = 0;
    // todo: add more ZTS there
    if(Z_TYPE_P(group) == IS_STRING) {
        g = getgrnam(Z_STRVAL_P(group));
    } else if(Z_TYPE_P(group) == IS_LONG) {
        g = getgrgid((uid_t)Z_LVAL_P(group));
    } else {
        zend_throw_exception(ion_class_entry(InvalidArgumentException), ERR_ION_PROCESS_INVALID_GID, 0);
        return;
    }

    if(!g) {
        RETURN_FALSE;
    }

    array_init(return_value);
    array_init(&members);

    for (count = 0; g->gr_mem[count] != NULL; count++) {
        add_next_index_string(&members, g->gr_mem[count]);
    }

    add_assoc_long  (return_value, "gid",     g->gr_gid);
    add_assoc_string(return_value, "name",    g->gr_name);
    add_assoc_zval(return_value,   "members", &members);

}

METHOD_ARGS_BEGIN(ION_Process, getGroup, 0)
    METHOD_ARG(group, 0)
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

    pw = ion_get_pw_by_zval(user);

    if(!pw) {
        if(errno) {
            if(Z_TYPE_P(user) == IS_STRING) {
                zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_NO_USER_INFO_NAMED, Z_STRVAL_P(user), strerror(errno));
            } else {
                zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_NO_USER_INFO_UID, Z_LVAL_P(user), strerror(errno));
            }
        } else {
            zend_throw_exception(ion_class_entry(InvalidArgumentException), ERR_ION_PROCESS_INVALID_UID, 0);
        }
        return;
    }

    array_init(return_value);

    add_assoc_long  (return_value, "uid",    pw->pw_uid);
    add_assoc_string(return_value, "name",   pw->pw_name);
    add_assoc_string(return_value, "gecos",  pw->pw_gecos);
    add_assoc_long  (return_value, "gid",	 pw->pw_gid);
    add_assoc_string(return_value, "dir",    pw->pw_dir);
    add_assoc_string(return_value, "shell",  pw->pw_shell);

    if(set_group && setgid(pw->pw_gid)) {
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, 0, ERR_ION_PROCESS_GET_GID, (int)pw->pw_gid, strerror(errno));
        return;
    }

    if(setuid(pw->pw_uid)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_GET_UID, (int)pw->pw_gid, strerror(errno));
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
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_GET_PRIO, pid, strerror(errno));
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
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_PROCESS_INVALID_PRIO, 0);
        return;
    }

    errno = 0;
    if (setpriority(PRIO_PROCESS, (id_t)pid, (int)priority)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_PRIO_FAILED, (int)pid, (int)priority, strerror(errno));
        return;
    }

    RETURN_LONG(prev_priority);
}

METHOD_ARGS_BEGIN(ION_Process, setPriority, 1)
    METHOD_ARG_LONG(priority, 0)
    METHOD_ARG_LONG(pid, 0)
METHOD_ARGS_END()

/** public function ION\Process::exec(string $command, array $options = []) : int */
CLASS_METHOD(ION_Process, exec) {
    zend_string      * command = NULL;
    zval             * options = NULL;
    int                pid;
    int                out_pipes[2];
    int                err_pipes[2];
    ion_process_exec * exec;
    zval               zexec;
    char             * env[2];
    struct passwd    * pw = NULL;
    char             * line;
    zend_bool          set_group = 0;
    zval             * zuser = NULL;
    zval             * zgroup = NULL;
    zval             * zpid = NULL;
    zval             * zcwd = NULL;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_STR(command)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_EX(options, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    spprintf(&line, 255, "_ION_EXEC_LINE=%s:%d", zend_get_executed_filename(), zend_get_executed_lineno());
    env[0] = line;
    env[1] = NULL;
    if(options) {
        zuser = zend_hash_str_find(Z_ARRVAL_P(options), STRARGS("user"));
        if(zuser) {
            pw = ion_get_pw_by_zval(zuser);
            if(!pw && errno) {
                if(Z_TYPE_P(zuser) == IS_STRING) {
                    zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_NO_USER_INFO_NAMED, Z_STRVAL_P(zuser), strerror(errno));
                } else {
                    zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_NO_USER_INFO_UID, Z_LVAL_P(zuser), strerror(errno));
                }
                return;
            }
            zgroup = zend_hash_str_find(Z_ARRVAL_P(options), STRARGS("set_group"));
            if(zend_hash_str_exists(Z_ARRVAL_P(options), STRARGS("set_group"))) {
                zval zgroup_bool;
                ZVAL_COPY_VALUE(&zgroup_bool, zgroup);
                convert_to_boolean_ex(&zgroup_bool);
                if(Z_ISTRUE(zgroup_bool)) {
                    set_group = true;
                }
            }
        }
        zcwd = zend_hash_str_find(Z_ARRVAL_P(options), STRARGS("cwd"));
    }

    if(pipe(out_pipes)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_EXEC_NO_STDOUT, strerror(errno));
        return;
    }
    if(pipe(err_pipes)) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_EXEC_NO_STDERR, strerror(errno));
        return;
    }

    pid = fork();
    if(pid == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), 0, ERR_ION_PROCESS_EXEC_FORK_FAIL, strerror(errno));
        return;
    } else if(pid) { // parent
        efree(line);
        if(options) {
            zpid = zend_hash_str_find(Z_ARRVAL_P(options), STRARGS("pid"));
            if(zpid && Z_ISREF_P(zpid)) {
                ZVAL_DEREF(zpid);
                zval_ptr_dtor(zpid);
                ZVAL_LONG(zpid, pid);
            }
        }
        close(out_pipes[1]);
        close(err_pipes[1]);

        object_init_ex(&zexec, ion_ce_ION_Process_Exec);

        zend_hash_index_add(GION(proc_execs), (zend_ulong) pid, &zexec);

        pion_update_property_str(ION_Process_Exec, &zexec, "command", zend_string_copy(command));
        exec            = get_instance(&zexec, ion_process_exec);

        exec->pid       = pid;
        exec->err       = bufferevent_socket_new(GION(base), err_pipes[0], BEV_OPT_CLOSE_ON_FREE);
        exec->out       = bufferevent_socket_new(GION(base), out_pipes[0], BEV_OPT_CLOSE_ON_FREE);
        exec->cancel_signal = SIGTERM;

        bufferevent_enable(exec->out, EV_READ);
        bufferevent_enable(exec->err, EV_READ);
        exec->deferred = ion_promisor_deferred_new_ex(NULL);
        ion_promisor_set_object_ptr(exec->deferred, exec, NULL);
        ion_object_addref(exec->deferred);
        RETURN_ION_OBJ(exec->deferred);
    } else {  // child
        if (dup2( out_pipes[1], 1 ) < 0 ) {
            perror(strerror(errno));
            _exit(1);
        }
        if (dup2( err_pipes[1], 2 ) < 0 ) {
            perror(strerror(errno));
            _exit(1);
        }
        if(pw) {
            if(set_group && setgid(pw->pw_gid)) {
                fprintf(stderr, ERR_ION_PROCESS_EXEC_SET_GID_FAIL, (int)pw->pw_gid, strerror(errno));
            }

            if(setuid(pw->pw_uid)) {
                fprintf(stderr, ERR_ION_PROCESS_EXEC_SET_UID_FAIL, (int)pw->pw_uid, strerror(errno));
            }


        }
        if(zcwd && chdir(Z_STRVAL_P(zcwd)) == FAILURE) {
            fprintf(stderr, ERR_ION_PROCESS_EXEC_CHDIR_FAIL, Z_STRVAL_P(zcwd), strerror(errno));
        }
        if(execle("/bin/sh", "sh", "-c", command->val, NULL, env)) {
            perror(strerror(errno));
        }
        _exit(1);
    }
}

METHOD_ARGS_BEGIN(ION_Process, exec, 1)
    METHOD_ARG(command, 0)
    METHOD_ARG_ARRAY(options, 0, 0)
METHOD_ARGS_END()



/** public function ION\Process::hasChildProcess(int $pid) : bool */
CLASS_METHOD(ION_Process, hasChildProcess) {
    zend_long pid = 0;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_LONG(pid)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(zend_hash_index_exists(GION(proc_childs), (zend_ulong) pid)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_ARGS_BEGIN(ION_Process, hasChildProcess, 1)
    ARGUMENT(pid, IS_LONG)
METHOD_ARGS_END()


/** public function ION\Process::getChildProcess(int $pid) : ION\Process\ChildProcess */
CLASS_METHOD(ION_Process, getChildProcess) {
    zend_long pid = 0;
    zval * child = NULL;

    ZEND_PARSE_PARAMETERS_START(1,2)
       Z_PARAM_LONG(pid)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    child = zend_hash_index_find(GION(proc_childs), (zend_ulong) pid);
    if(child) {
        ZVAL_COPY(return_value, child);
    } else {
        RETURN_NULL();
    }
}

METHOD_ARGS_BEGIN(ION_Process, getChildProcess, 1)
    ARGUMENT(pid, IS_LONG)
METHOD_ARGS_END()

/** public function ION\Process::getChilds() : ION\Process\ChildProcess[] */
CLASS_METHOD(ION_Process, getChildProcesses) {
    RETURN_ARR(zend_array_dup(GION(proc_childs)));
}

METHOD_WITHOUT_ARGS_RETURN_ARRAY(ION_Process, getChildProcesses);


/** public function ION\Process::hasParentIPC() : bool */
CLASS_METHOD(ION_Process, hasParentIPC) {
    if(GION(parent_ipc)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS_RETURN_BOOL(ION_Process, hasParentIPC)

/** public function ION\Process::getParentIPC() : ION\Process\IPC */
CLASS_METHOD(ION_Process, getParentIPC) {
    if(GION(parent_ipc)) {
        ion_object_addref(GION(parent_ipc));
        RETURN_ION_OBJ(GION(parent_ipc));
    } else {
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Process, getParentIPC);

#undef stdin
#undef stdout
#undef stderr


/** public function ION\Process::stdin() : Stream */
CLASS_METHOD(ION_Process, stdin) {
    zend_object_addref(GION(input));
    RETURN_OBJ(GION(input));
}

METHOD_WITHOUT_ARGS(ION_Process, stdin);

/** public function ION\Process::stdout() : Stream */
CLASS_METHOD(ION_Process, stdout) {
    zend_object_addref(GION(output));
    RETURN_OBJ(GION(output));
}

METHOD_WITHOUT_ARGS(ION_Process, stdout);

/** public function ION\Process::stderr() : Stream */
CLASS_METHOD(ION_Process, stderr) {
    zend_object_addref(GION(error));
    RETURN_OBJ(GION(error));
}

METHOD_WITHOUT_ARGS(ION_Process, stderr);

CLASS_METHODS_START(ION_Process)
    METHOD(ION_Process, fork,              ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, kill,              ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, signal,            ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, clearSignal,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPid,            ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getParentPid,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getUser,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getGroup,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setUser,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getPriority,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, setPriority,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, hasChildProcess,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getChildProcess,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getChildProcesses, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, hasParentIPC,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, getParentIPC,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, exec,              ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, stdin,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, stdout,            ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process, stderr,            ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Process) {
    zval signals;
    zend_array * ht = (HashTable *) pemalloc(sizeof(HashTable), 1);
    zend_hash_init(ht, 64, NULL, zval_dtor_wrapper, 1);
//    array_init(&signals);

    ZVAL_ARR(&signals, ht);
    zval_add_ref(&signals);
//    signals.value.arr = ht;

    PION_REGISTER_STATIC_CLASS(ION_Process, "ION\\Process");

    PION_REGISTER_VOID_EXTENDED_CLASS(ION_ProcessException, ion_ce_ION_RuntimeException, "ION\\ProcessException");


    /* Signal Constants */

    add_assoc_long(&signals, "HUP",   (zend_long) SIGHUP);
    add_assoc_long(&signals, "INT",   (zend_long) SIGINT);
    add_assoc_long(&signals, "QUIT",  (zend_long) SIGQUIT);
    add_assoc_long(&signals, "ILL",   (zend_long) SIGILL);
    add_assoc_long(&signals, "TRAP",  (zend_long) SIGTRAP);
    add_assoc_long(&signals, "ABRT",  (zend_long) SIGABRT);
#ifdef SIGIOT
    add_assoc_long(&signals, "IOT",   (zend_long) SIGIOT);
#endif

    add_assoc_long(&signals, "BUS",   (zend_long) SIGBUS);
    add_assoc_long(&signals, "FPE",   (zend_long) SIGFPE);
    add_assoc_long(&signals, "KILL",  (zend_long) SIGKILL);
    add_assoc_long(&signals, "USR1",  (zend_long) SIGUSR1);
    add_assoc_long(&signals, "SEGV",  (zend_long) SIGSEGV);
    add_assoc_long(&signals, "USR2",  (zend_long) SIGUSR2);
    add_assoc_long(&signals, "PIPE",  (zend_long) SIGPIPE);
    add_assoc_long(&signals, "ALRM",  (zend_long) SIGALRM);
    add_assoc_long(&signals, "TERM",  (zend_long) SIGTERM);
#ifdef SIGSTKFLT
    add_assoc_long(&signals, "STKFLT",(zend_long) SIGSTKFLT);
#endif
#ifdef SIGCLD
    add_assoc_long(&signals, "CLD",   (zend_long) SIGCLD);
#endif

#ifdef SIGCHLD
    add_assoc_long(&signals, "CHLD",  (zend_long) SIGCHLD);
#endif
    add_assoc_long(&signals, "CONT",  (zend_long) SIGCONT);
    add_assoc_long(&signals, "STOP",  (zend_long) SIGSTOP);
    add_assoc_long(&signals, "TSTP",  (zend_long) SIGTSTP);
    add_assoc_long(&signals, "TTIN",  (zend_long) SIGTTIN);
    add_assoc_long(&signals, "TTOU",  (zend_long) SIGTTOU);
    add_assoc_long(&signals, "URG",   (zend_long) SIGURG );
    add_assoc_long(&signals, "XCPU",  (zend_long) SIGXCPU);
    add_assoc_long(&signals, "XFSZ",  (zend_long) SIGXFSZ);
    add_assoc_long(&signals, "VTALRM",(zend_long) SIGVTALRM);
    add_assoc_long(&signals, "PROF",  (zend_long) SIGPROF);
    add_assoc_long(&signals, "WINCH", (zend_long) SIGWINCH);
#ifdef SIGPOLL
    add_assoc_long(&signals, "POLL",  (zend_long) SIGPOLL);
#endif
    add_assoc_long(&signals, "IO",    (zend_long) SIGIO);
#ifdef SIGPWR
    add_assoc_long(&signals, "PWR",   (zend_long) SIGPWR);
#endif
#ifdef SIGSYS
    add_assoc_long(&signals, "SYS",   (zend_long) SIGSYS);
    add_assoc_long(&signals, "BABY",  (zend_long) SIGSYS);
#endif
    add_assoc_long(&signals, "_IGN",  (zend_long) SIG_IGN);
    add_assoc_long(&signals, "_DFL",  (zend_long) SIG_DFL);
    add_assoc_long(&signals, "ERR",  (zend_long) SIG_ERR);

    PION_CLASS_CONST_ZVAL(ION_Process, "SIG", &signals);
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Process) {
    ALLOC_HASHTABLE(GION(signals));
    zend_hash_init(GION(signals), 128, NULL, NULL, 0);

    ALLOC_HASHTABLE(GION(proc_childs));
    zend_hash_init(GION(proc_childs), 32, NULL, zval_ptr_dtor_wrapper, 0);

    ALLOC_HASHTABLE(GION(proc_execs));
    zend_hash_init(GION(proc_execs), 64, NULL, zval_ptr_dtor_wrapper, 0);

    GION(sigchld) = evsignal_new(GION(base), SIGCHLD, ion_process_sigchld, NULL);
    evsignal_add(GION(sigchld), NULL);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Process) {
    evsignal_del(GION(sigchld));

    if(GION(parent_ipc)) {
        ion_object_release(GION(parent_ipc));
    }

    zend_hash_clean(GION(proc_childs));
    zend_hash_destroy(GION(proc_childs));
    FREE_HASHTABLE(GION(proc_childs));

    zend_hash_clean(GION(proc_execs));
    zend_hash_destroy(GION(proc_execs));
    FREE_HASHTABLE(GION(proc_execs));

    GION(signals)->pDestructor = _ion_process_signal_dtor;
    zend_hash_clean(GION(signals));
    zend_hash_destroy(GION(signals));
    FREE_HASHTABLE(GION(signals));
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Process) {
    return SUCCESS;
}