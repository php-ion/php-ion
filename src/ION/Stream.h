#ifndef ION_STREAM_H
#define ION_STREAM_H

#include "../pion.h"
#include <event.h>

#define ION_STREAM_FLAG_SOCKET  1
#define ION_STREAM_FLAG_PAIR    2
#define ION_STREAM_FLAG_READING 16
#define ION_STREAM_FLAG_FLUSHED 32

#define ION_STREAN_LN_TRIM_TOKEN         1
#define ION_STREAN_LN_TRIM_WITH_TOKEN    2
#define ION_STREAN_LN_TRIM_WITHOUT_TOKEN 4

typedef struct bufferevent bevent;

typedef struct _ion_stream {
    zend_object      std;
    short            flags;   // flags ION_STREAM_FLAG_*
    bevent         * buffer;  // input and output bufferevent
    char           * token;   // token for awaitLine
    int              token_length; // token length
    long             length;  // bytes for reading
    zval           * read;    // read deferred object
    zval           * flush;   // flush deferred object
    zval           * connect; // connection deferred object
    pionCb         * on_data;
    pionCb         * on_close;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_stream;

//#ifdef ZTS
//#define STREAM_BUFFER_DEFAULT_FLAGS BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE
//#else
#define STREAM_BUFFER_DEFAULT_FLAGS BEV_OPT_DEFER_CALLBACKS
//#endif

#define CHECK_STREAM(stream)

DEFINE_CLASS(ION_Stream);
CLASS_INSTANCE_DTOR(ION_Stream);
CLASS_INSTANCE_CTOR(ION_Stream);

#endif //ION_STREAM_H
