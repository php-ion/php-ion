#ifndef PION_INIT_H
#define PION_INIT_H

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <php.h>
#include <event.h>
#include "callback.h"

#ifdef PHP_WIN32
# define ION_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define ION_API __attribute__ ((visibility("default")))
#else
# define ION_API
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#if defined(COMPILE_DL_ION) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

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

typedef struct _ion_global_queue ion_global_queue;

#ifndef true
# define true 1
#endif

#ifndef false
# define false 0
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

typedef struct _zend_ion_global_stats {
    ion_time   start_time;
    ion_time   active_time;
    zend_ulong handeled_events;
} zend_ion_global_stats;

ZEND_BEGIN_MODULE_GLOBALS(ion)
    // base
    ion_event_base   * base;    // event base
    ion_event_config * config;  // event config
    uint               flags;
    HashTable        * timers;  // array of timers

    // Stats
    zend_bool  stats_enabled;
    ion_time   start_time;
    ion_time   active_time;
    zend_ulong handeled_events;

    // Stream
    zend_ulong    stream_index;
    zend_object * input;
    zend_object * output;
    zend_object * error;

    // DNS
    ion_evdns_base * evdns;      // event dns base
    HashTable      * resolvers;  // resolve requests
    char           * resolv_conf;
    zend_long        resolv_options;
    char           * hosts_file;

    // Process
    HashTable   * signals;     // registered signals
    HashTable   * childs;      // spawned child processes
    HashTable   * workers;     // spawned workers
    HashTable   * disconnected;  // list of disconnected workers
    zend_object * master;      // link to master process
    ion_event   * sigchld;

    // FS
    int         watch_fd;    // inotify or kqueue file descriptor
    ion_event * watch_event; // watch_fd listener
    HashTable * watchers;    // list of listened filenames

    // SSL
    int ssl_index;

    // Misc.
    ion_global_queue * queue;
    zend_bool     define_metrics;
    zend_object * quit_marker;
    zend_ion_global_cache * cache; // do not to change cache at runtime!
ZEND_END_MODULE_GLOBALS(ion)

ZEND_EXTERN_MODULE_GLOBALS(ion);

// ION globals access
#define GION(v) ZEND_MODULE_GLOBALS_ACCESSOR(ion, v)


static zend_always_inline zend_bool ion_stats_begin() {
    if(GION(stats_enabled)) {

        return true;
    } else {
        return false;
    }
}

static zend_always_inline void ion_stats_commit() {

}

static zend_always_inline void ion_cb_commit() {
    if(EG(exception)) {
        event_base_loopbreak(GION(base));
    }
}


#define ION_CB_BEGIN() zend_bool __stats = ion_stats_begin();

#define ION_CB_END()   \
    if(__stats) {           \
        ion_stats_commit(); \
    }                       \
    ion_cb_commit();

#endif //PION_INIT_H
