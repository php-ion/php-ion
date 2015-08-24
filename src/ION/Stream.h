#ifndef ION_STREAM_H
#define ION_STREAM_H

#include "../pion.h"
#include <event.h>

#define ION_STREAM_FLAG_SOCKET  1
#define ION_STREAM_FLAG_READING 16
#define ION_STREAM_FLAG_FLUSHED 32

typedef struct bufferevent bevent;

typedef struct _ion_stream {
    zend_object      std;
    short            flags;
    bevent         * buffer;
    zval           * read;
    zval           * flush;
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


DEFINE_CLASS(ION_Stream);
CLASS_INSTANCE_DTOR(ION_Stream);
CLASS_INSTANCE_CTOR(ION_Stream);

#endif //ION_STREAM_H
