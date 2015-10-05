
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "config.h"
#include "pion/exceptions.h"
#include "pion/debug.h"
#include "pion/linkedlist.h"
#include "pion/callback.h"
#include "pion/engine.h"
#include "pion/deferred.h"
#include "pion/net.h"

typedef struct _ion_dns {
    struct evdns_base * evdns;
    HashTable         * requests;
} ion_dns;

typedef struct _ion_proc {
    HashTable * signals;          // array of listening signals
    HashTable * execs;            //
    HashTable * childs;           // array of process handlers
} ion_proc;

/** main structure */
typedef struct _ion_base {
    struct event_base   * base;     // event base
    struct event_config * config; // event config
    ion_dns             * dns;    // event DNS base
    ion_proc            * proc;
    long  i;                     // internal counter of timers
    HashTable *timers;           // array of timers
//    struct event *sigsegv;
//    pion_llist *queue;                // queue of defers object
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_base;

extern ion_base *ionBase;

#define ION(prop) \
    ionBase->prop

#define ion_loop_break() event_base_loopbreak(ION(base))

#define ION_CHECK_LOOP()                 \
    if(EG(exception)) {                  \
        event_base_loopbreak(ION(base)); \
    }

#define ION_CHECK_LOOP_RETURN()          \
    if(EG(exception)) {                  \
        event_base_loopbreak(ION(base)); \
        return;                          \
    }

#define ION_EVCB_START()

#define ION_EVCB_END()

#define ION_EVCB_RETURN()

#define SET_TIMEVAL(tval, dval)                          \
    tval.tv_usec = (int)((int)(dval*1000000) % 1000000); \
    tval.tv_sec = (int)dval;
/**
 * For debug
 */

#endif //ION_FRAMEWORK_H
