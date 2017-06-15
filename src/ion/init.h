#ifndef PION_INIT_H
#define PION_INIT_H

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <php.h>
#include <event.h>
//#include "callback.h"

#ifdef PHP_WIN32
# define ION_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define ION_API __attribute__ ((visibility("default")))
#else
# define ION_API
#endif

#ifdef ZTS
# include "TSRM.h"
#else
BEGIN_EXTERN_C()
ZEND_API extern int compiler_globals_id;
ZEND_API extern int executor_globals_id;
END_EXTERN_C()
#endif

#if defined(COMPILE_DL_ION) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

// external typedef
typedef struct event          ion_event;
typedef struct evconnlistener ion_evlistener;
typedef struct evbuffer       ion_evbuffer;
typedef struct bufferevent    ion_buffer;
typedef struct event_base     ion_event_base;
typedef struct event_config   ion_event_config;
typedef struct evdns_base     ion_evdns_base;
typedef struct timeval        ion_time;
typedef struct ev_token_bucket_cfg          ion_rate_limit_cfg;
typedef struct bufferevent_rate_limit_group ion_rate_limit;
typedef struct skiplist       ion_skiplist;

// core typedef
typedef struct _ion_global_queue ion_global_queue;

// internal typedef
typedef struct _ion_promisor ion_promisor;
typedef struct _ion_promisor_action_cb ion_promisor_action_cb;

typedef struct _ion_crypto ion_crypto;
typedef struct _ion_listener ion_listener;
typedef struct _ion_stream_token ion_stream_token;
typedef struct _ion_stream ion_stream;

typedef struct _ion_process_ipc ion_process_ipc;
typedef struct _ion_process_exec ion_process_exec;
typedef struct _ion_process_child ion_process_child;
typedef struct _ion_process_signal ion_process_signal;

typedef struct _ion_fs_watcher ion_fs_watcher;

typedef struct _ion_dns_addr_request ion_dns_addr_request;

typedef struct _ion_uri ion_uri;
typedef struct _ion_http_parser ion_http_parser;
typedef struct _ion_http_message ion_http_message;
typedef struct _ion_http_request ion_http_request;
typedef struct _ion_http_response ion_http_response;
typedef struct _ion_http_websocket_frame ion_http_websocket_frame;
typedef struct _ion_http_multi_parted_parser ion_http_multi_parted_parser;
typedef struct _ion_websocket_parser ion_websocket_parser;
typedef struct _ion_http_body_parser ion_http_body_parser;

#ifndef true
# define true 1
#endif

#ifndef false
# define false 0
#endif

#if PHP_API_VERSION == 20151012
# define IS_PHP70 1
#else

#endif


#define SET_TIMEVAL(tval, dval) SET_TIMEVAL_P(&tvalp, dval)

#define SET_TIMEVAL_P(tvalp, dval)                      \
    (tvalp)->tv_usec = (int)((dval)*1000000) % 1000000; \
    (tvalp)->tv_sec = (int)(dval);

#define STRARGS(str) str, sizeof(str) - 1

typedef struct _zend_ion_global_cache {
    HashTable   * index;
    zend_string * interned_strings[512];
} zend_ion_global_cache;

ZEND_BEGIN_MODULE_GLOBALS(ion)
    // base
    ion_event_base   * base;    // event base
    ion_event_config * config;  // event config
    uint               flags;
    zend_array       * timers;  // array of timers

    // Stats
    zend_bool  stats_enabled;
#ifdef ION_DEBUG
    zend_bool  in_call;       // callback started
#endif
    double     php_time;
    ion_time   reset_ts;
    zend_ulong calls_count;

    // Stream
    zend_ulong    stream_index;
    zend_object * input;
    zend_object * output;
    zend_object * error;

    // DNS
    ion_evdns_base * evdns;      // event dns base
    zend_bool        adns_enabled;
    zend_array     * resolvers;  // resolve requests
    char           * resolv_conf;
    zend_long        resolv_options;
    char           * hosts_file;

    // Process
    zend_array      * signals;      // registered signals
    zend_array      * proc_execs;   // exec processes
    zend_array      * proc_childs;  // spawned child processes
    zend_array      * workers;      // spawned workers
    zend_array      * disconnected; // list of disconnected workers
    ion_process_ipc * parent_ipc;   // link to IPC of parent process
    ion_event       * sigchld;

    // FS
    int          watch_fd;    // inotify or kqueue file descriptor
    ion_event  * watch_event; // watch_fd listener
    zend_array * watchers;    // list of listened filenames

    // SSL
    int ssl_index;

    // Misc.
    ion_global_queue * queue;
    zend_bool     define_metrics;
    zend_ion_global_cache * cache; // do not to change cache at runtime!
ZEND_END_MODULE_GLOBALS(ion)

ZEND_EXTERN_MODULE_GLOBALS(ion);

// ION globals access
#define GION(v) ZEND_MODULE_GLOBALS_ACCESSOR(ion, v)

static zend_always_inline void ion_stats_commit(ion_time * start_time) {
    ion_time vt = { 0,0 };
    gettimeofday(&vt, NULL);
    timersub(&vt, start_time, &vt);
    GION(calls_count) += 1; // do not use ++ because thread's magic
    GION(php_time) += vt.tv_sec + vt.tv_usec / 1e6;
}

static zend_always_inline void ion_cb_commit() {
    if(EG(exception)) {
        event_base_loopbreak(GION(base));
    }
}


#define ION_CB_BEGIN() \
    ion_time __ion_call_start;  \
    if(GION(stats_enabled)) {   \
        gettimeofday(&__ion_call_start, NULL); \
    }


#define ION_CB_END()                         \
    if(timerisset(&__ion_call_start)) {      \
        ion_stats_commit(&__ion_call_start); \
    }                                        \
    ion_cb_commit();

#endif //PION_INIT_H
