
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "config.h"
#include "pion/exceptions.h"
#include "pion/debug.h"
#include "pion/callback.h"
#include "pion/engine.h"
#include "pion/deferred.h"
#include "pion/net.h"

/** main structure */
typedef struct _ion_base {
    struct event_base *base;     // event base
    struct evdns_base *evdns;    // event DNS base
    struct event_config *config; // event config
//    zval *dns;                   // DNS instance
    long  i;                     // internal counter of timers
    HashTable *signals;          // array of listening signals
    HashTable *timers;           // array of timers
    HashTable *execs;            // array of process childs
//    struct event *sigsegv;
//    pion_llist *queue;                // queue of defers object
#ifdef ZTS
    void ***thread_ctx;
#endif
} IONBase;

extern IONBase *ionBase;

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
