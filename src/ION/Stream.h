#ifndef ION_STREAM_H
#define ION_STREAM_H

#include "../pion.h"
#include <event.h>

BEGIN_EXTERN_C();

// ** state flags begin **
// stream types
#define ION_STREAM_STATE_SOCKET    1<<0
#define ION_STREAM_STATE_PAIR      1<<1 // use with ION_STREAM_STATE_SOCKET
#define ION_STREAM_STATE_PIPE      1<<2
#define ION_STREAM_STATE_FILE      1<<3 // reserved

// reading and writing states
#define ION_STREAM_STATE_READING   1<<4
#define ION_STREAM_STATE_FLUSHED   1<<5
#define ION_STREAM_STATE_HAS_DATA  1<<6

// behavior flags
#define ION_STREAM_STATE_CLOSE_ON_FLUSH   1<<8

// runtime stream states
#define ION_STREAM_STATE_ENABLED   1<<9
#define ION_STREAM_STATE_CONNECTED 1<<10
#define ION_STREAM_STATE_EOF       1<<11
#define ION_STREAM_STATE_ERROR     1<<12
#define ION_STREAM_STATE_SHUTDOWN  1<<13
#define ION_STREAM_STATE_CLOSED    (ION_STREAM_STATE_EOF | ION_STREAM_STATE_ERROR | ION_STREAM_STATE_SHUTDOWN)

#define ION_STREAM_NAME_IPV4       1<<14
#define ION_STREAM_NAME_IPV6       1<<15
#define ION_STREAM_NAME_UNIX       1<<16
#define ION_STREAM_NAME_MASK       (ION_STREAM_NAME_IPV4 | ION_STREAM_NAME_IPV6 | ION_STREAM_NAME_UNIX)

// ** state flags end **

// mode for line reading (getLine() and awaitLine())
#define ION_STREAM_MODE_TRIM_TOKEN    1
#define ION_STREAM_MODE_WITH_TOKEN    2
#define ION_STREAM_MODE_WITHOUT_TOKEN 4

#define ION_STREAM_TOKEN_MODE_MASK    (ION_STREAM_MODE_TRIM_TOKEN | ION_STREAM_MODE_WITH_TOKEN | ION_STREAM_MODE_WITHOUT_TOKEN)
#define ION_STREAM_TOKEN_LIMIT        8

#define ION_STREAM_NAME_HOST          0
#define ION_STREAM_NAME_ADDRESS       1
#define ION_STREAM_NAME_PORT          2
#define ION_STREAM_NAME_SHORT_MASK    3


typedef struct bufferevent bevent;

typedef struct _ion_stream_token {
    char           * token;   // token string
    long             token_length; // strlen of token
    long             length;
    long             offset;
    short            flags;
    long             position;
} ion_stream_token;

const ion_stream_token empty_stream_token = { NULL, 0, 0, 0, ION_STREAM_MODE_TRIM_TOKEN, -1 };

typedef struct _ion_stream {
    zend_object        std;
    uint               state;   // flags
    bevent           * buffer;  // input and output bufferevent
    size_t             length;  // bytes for reading
    size_t             input_size;
    ion_stream_token * token;
    zval             * self;
    zval             * read;    // read deferred object
    zval             * flush;   // state deferred object
    zval             * connect; // connection deferred object
    zval             * closing; // closing deferred object
    pionCb           * on_data;
    char             * name_self;    // cache of getsockname
    char             * name_remote;  // cache of getpeername
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_stream;

//#ifdef ZTS
//#define STREAM_BUFFER_DEFAULT_FLAGS BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE
//#else
#define STREAM_BUFFER_DEFAULT_FLAGS BEV_OPT_DEFER_CALLBACKS
//#endif

#define ion_stream_new(buffer, state)                  _ion_stream_new(buffer, state, NULL TSRMLS_CC)
#define ion_stream_new_ex(buffer, state, class_entry)  _ion_stream_new(buffer, state, class_entry TSRMLS_CC)
#define ion_stream_zval(zstream, buffer, state, class_entry)    _ion_stream_zval(zstream, buffer, state, NULL TSRMLS_CC)
#define ion_stream_zval_ex(zstream, buffer, state, class_entry) _ion_stream_zval(zstream, buffer, state, class_entry TSRMLS_CC)

int    _ion_stream_zval(zval * zstream, bevent * buffer, int flags, zend_class_entry * cls TSRMLS_DC);
zval * _ion_stream_new(bevent * buffer, int flags, zend_class_entry * cls TSRMLS_DC);

ion_define_class_entry(ION_Streams);
CLASS_INSTANCE_DTOR(ION_Stream);
CLASS_INSTANCE_CTOR(ION_Stream);

#define ION_Stream_RuntimeException() spl_ce_RuntimeException

END_EXTERN_C();

#endif //ION_STREAM_H
