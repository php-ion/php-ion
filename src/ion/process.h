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

typedef enum _ion_process_child_type {
    ION_PROC_CHILD_EXEC   = 1 << 0,
    ION_PROC_CHILD_WORKER = 1 << 1,
} ion_process_child_type;

typedef struct _ion_process_exec {
    zend_object   std;
    uint          flags;
    pid_t         pid;
    ion_buffer  * out;
    ion_buffer  * err;
    ion_buffer  * in;
    zend_object * deferred;
} ion_process_exec;

enum ion_spawn_flags {
    ION_WORKER_RESET_SIGNALS   = 1 << 0,
    ION_WORKER_RESET_INTERVALS = 1 << 1,
};

enum ion_worker_flags {
    ION_WORKER_DONE         = 1 << 2,
    ION_WORKER_FAILED       = 1 << 3,
    ION_WORKER_SIGNALED     = 1 << 4,
    ION_WORKER_ABORT        = 1 << 5,
    ION_WORKER_FINISHED     = ION_WORKER_DONE | ION_WORKER_FAILED | ION_WORKER_SIGNALED | ION_WORKER_ABORT,
    ION_WORKER_STARTED      = 1 << 6,
    ION_WORKER_MASTER       = 1 << 7,
    ION_WORKER_CHILD        = 1 << 8,
    ION_WORKER_DISCONNECTED = 1 << 8,
};

typedef struct _ion_process_child {
    zend_object   std;
    uint          flags;
    pid_t         pid;
} ion_process_child;

typedef struct _ion_process_worker {
    zend_object   std;
    uint32_t      flags;
    pid_t         pid;
    int           exit_status;
    ion_time      started_time;
    ion_buffer  * buffer;
    int           signal;
    pion_cb     * cb;
    zend_object * on_message;
    zend_object * on_exit;
    websocket_parser * parser;
} ion_process_worker;


#define ion_process_exec_object(pz) object_init_ex(pz, ion_ce_ION_Process_ExecResult)
#define ion_procces_is_exec(obj) (get_object_instance(obj, ion_process_child)->flags & ION_PROC_CHILD_EXEC)

void ion_process_exec_disconnect(ion_buffer * b, short what, void * ctx);

void ion_process_sigchld(evutil_socket_t signal, short flags, void * arg);
void ion_process_add_child(pid_t pid, ion_process_child_type type, zend_object * object);
void ion_process_exec_exit(zend_object * exec, int status);
void ion_process_worker_exit(zend_object * worker, int status);
void ion_process_exec_dtor(zend_object * exec);
void ion_process_worker_dtor(zend_object * worker);
void ion_process_child_dtor(zval * child);

END_EXTERN_C();

#endif //ION_CORE_PROCESS_H
