#ifndef ION_CORE_PROCESS_H
#define ION_CORE_PROCESS_H

BEGIN_EXTERN_C();

extern ION_API zend_class_entry * ion_ce_ION_Process;
extern ION_API zend_class_entry * ion_ce_ION_Process_ExecResult;

extern ION_API struct passwd * ion_get_pw_by_zval(zval * zuser);

typedef struct _ion_exec {
    zend_string * command;
    uint          flags;
    int           pid;
    int           stdout_fd;
    int           stderr_fd;
    int           stdin_fd;
    ion_buffer  * out;
    ion_buffer  * err;
    ion_buffer  * in;
    zend_object * deferred;
} ion_exec;

END_EXTERN_C();

#endif //ION_CORE_PROCESS_H
