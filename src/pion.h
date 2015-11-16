
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "pion/init.h"
#include "pion/zts.h"
#include "pion/exceptions.h"
#include "pion/debug.h"
#include "pion/callback.h"
#include "pion/engine.h"
#include "pion/promisor.h"
#include "pion/net.h"
#include "pion/stream.h"

typedef struct event event;

typedef struct _ion_dns {
    struct evdns_base * evdns;
    HashTable         * requests;
} ion_dns;

typedef struct _ion_proc {
    HashTable * signals;          // array of listening signals
    HashTable * execs;            //
    HashTable * childs;           // array of process handlers
} ion_proc;

#define ION_IN_LOOP 1

/** main structure */
typedef struct _ion_base {
    struct event_base   * base;    // event base
    struct event_config * config;  // event config
    uint                  flags;
    ion_dns             * dns;     // event DNS base
    ion_proc            * proc;
    long  i;                       // internal counter of timers
    HashTable           * timers;  // array of timers
    HashTable           * signals; // array of timers
//    struct event *sigsegv;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_base;

ZEND_API zend_bool ion_reinit();
ZEND_API void * php_emalloc_wrapper(size_t size);
ZEND_API void * php_realloc_wrapper(void * nmemb, size_t size);
ZEND_API void   php_efree_wrapper(void * ptr);

extern ion_base *ionBase;

#endif //ION_FRAMEWORK_H
