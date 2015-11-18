#ifndef PION_INIT_H
#define PION_INIT_H

#include "config.h"
#include <php.h>
#include <event.h>
#include "engine.h"
#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(COMPILE_DL_ION) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

typedef struct event ion_event;
typedef struct bufferevent bevent;
typedef struct bufferevent_rate_limit_group buffer_group;
typedef struct evconnlistener ion_evlistener;
typedef struct evbuffer ion_evbuffer;
typedef struct evbuffer_cb_entry ion_evbuffer_cb;
typedef struct bufferevent ion_buffer;

#define ION(prop) \
    ionBase->prop

#define ion_loop_break() event_base_loopbreak(ION(base))

#define ION_LOOP_CB_BEGIN()
#define ION_LOOP_CB_END() ION_CHECK_LOOP()

#define ION_CHECK_LOOP()                 \
    if(EG(exception)) {                  \
        event_base_loopbreak(ION(base)); \
    }

#define SET_TIMEVAL(tval, dval)                          \
    tval.tv_usec = (int)((int)(dval*1000000) % 1000000); \
    tval.tv_sec = (int)dval;

#endif //PION_INIT_H
