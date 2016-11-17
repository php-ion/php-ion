#ifndef ION_CORE_PROCESS_H
#define ION_CORE_PROCESS_H

#include <deps/websocket-parser/websocket_parser.h>
#include "callback.h"

BEGIN_EXTERN_C();

extern ION_API zend_class_entry * ion_ce_ION_Process;
extern ION_API zend_class_entry * ion_ce_ION_ProcessException;
extern ION_API zend_class_entry * ion_ce_ION_Process_Exec;
extern ION_API zend_class_entry * ion_ce_ION_Process_ChildProcess;
extern ION_API zend_class_entry * ion_ce_ION_Process_IPC;
extern ION_API zend_class_entry * ion_ce_ION_Process_IPC_Message;

extern ION_API struct passwd * ion_get_pw_by_zval(zval * zuser);


// IPC object instance
typedef struct _ion_process_ipc {
    zend_object   std;
    uint32_t      flags;
    zval          ctx;
    ion_buffer  * buffer;
    zend_object * on_message;
    zend_object * on_disconnect;
    websocket_parser * parser;
    zend_string * frame_body;
} ion_process_ipc;


enum ion_process_flags {
    ION_PROCESS_EXEC         = 1 << 0,
    ION_PROCESS_CHILD        = 1 << 1,
    ION_PROCESS_DONE         = 1 << 2,
    ION_PROCESS_FAILED       = 1 << 3,
    ION_PROCESS_SIGNALED     = 1 << 4,
    ION_PROCESS_ABORT        = 1 << 5,
    ION_PROCESS_STARTED      = 1 << 6,

    ION_PROCESS_FINISHED     = ION_PROCESS_DONE | ION_PROCESS_FAILED | ION_PROCESS_SIGNALED | ION_PROCESS_ABORT,
};

typedef struct _ion_process_exec {
    zend_object   std;
    uint          flags;
    pid_t         pid;
    ion_buffer  * out;
    ion_buffer  * err;
    ion_buffer  * in;
    zend_object * deferred;
    int           cancel_signal;
} ion_process_exec;

typedef struct _ion_process_child {
    zend_object   std;
    uint          flags;
    pid_t         pid;
    pid_t         ppid; // parent pid for validation
    int           exit_status;
    ion_time      started_time;
    ion_process_ipc * ipc_parent;
    ion_process_ipc * ipc_child;
    int           signal;
    pion_cb     * on_start;
    zend_object * prom_exit;
    zend_object * prom_started;
} ion_process_child;


#define ION_IPC_CONNECTED 0x1
#define ION_IPC_CTX_RELEASE 0x2
// opcodes
#define ION_IPC_DATA    WS_OP_CONTINUE
// reserve
//#define ION_IPC_STREAM  WS_OP_TEXT
//#define ION_IPC_FD      WS_OP_BINARY
// WS_OP_CLOSE    = 0x8,
// WS_OP_PING     = 0x9,
//#define ION_IPC_LISTENER  0x10
// WS_OP_PONG     = 0xA,

// marks
#define ION_IPC_FIN  WS_FINAL_FRAME

#define ion_process_exec_object(pz) object_init_ex(pz, ion_ce_ION_Process_Exec)
#define ion_process_is_exec(obj) (get_object_instance(obj, ion_process_child)->flags & ION_PROCESS_EXEC)

void ion_process_sigchld(evutil_socket_t signal, short flags, void * arg);
void ion_process_exec_exit(zend_object * exec, int status);
void ion_process_child_exit(zend_object * worker, int status);

// IPC
int ion_process_ipc_message_begin(websocket_parser * parser);
int ion_process_ipc_message_body(websocket_parser * parser, const char * at, size_t length);
int ion_process_ipc_message_end(websocket_parser * parser);
void ion_process_ipc_incoming(ion_buffer * bev, void * ctx);

int ion_ipc_create(zval * one, zval * two, zval * ctx1, zval * ctx2, uint32_t flags);

END_EXTERN_C();

#endif //ION_CORE_PROCESS_H
