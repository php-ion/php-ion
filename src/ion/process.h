#ifndef ION_CORE_PROCESS_H
#define ION_CORE_PROCESS_H

#include <deps/websocket-parser/websocket_parser.h>
#include "callback.h"

BEGIN_EXTERN_C();

extern ION_API zend_class_entry * ion_ce_ION_Process;
extern ION_API zend_class_entry * ion_ce_ION_ProcessException;
extern ION_API zend_class_entry * ion_ce_ION_Process_ExecResult;
extern ION_API zend_class_entry * ion_ce_ION_Process_Worker;

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

enum ion_spawn_flags {
    ION_WORKER_RESET_SIGNALS   = 1 << 0,
    ION_WORKER_RESET_INTERVALS = 1 << 1,
};

enum ion_worker_flags {
    ION_WORKER_QUIT    = 1 << 0,
    ION_WORKER_FAILED  = 1 << 1,
    ION_WORKER_KILLER  = 1 << 2,
    ION_WORKER_STARTED = 1 << 3,
    ION_WORKER_MASTER  = 1 << 4,
    ION_WORKER_CHILD   = 1 << 5,
    ION_WORKER_FINISHED = ION_WORKER_QUIT | ION_WORKER_FAILED | ION_WORKER_KILLER,
};

typedef struct _ion_process_worker {
    zend_object   std;
    uint32_t      flags;
    ion_time      started_time;
    pid_t         pid;
    ion_buffer  * buffer;
    uint8_t       signal;
    int16_t       exit_status;
    pion_cb     * cb;
    zend_object * on_message;
    zend_object * on_exit;
    websocket_parser * parser;

} ion_process_worker;

END_EXTERN_C();

#endif //ION_CORE_PROCESS_H
