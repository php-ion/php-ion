#ifndef ION_PROCESS_H
#define ION_PROCESS_H

BEGIN_EXTERN_C();

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

#endif //ION_PROCESS_H
