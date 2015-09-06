#ifndef ION_STREAM_H
#define ION_STREAM_H

#include "../pion.h"
#include <event.h>

BEGIN_EXTERN_C();

// stream types
#define ION_STREAM_STATE_SOCKET    1<<0
#define ION_STREAM_STATE_PAIR      1<<1
#define ION_STREAM_STATE_PIPE      1<<2

// reading and writing states
#define ION_STREAM_STATE_READING   1<<4
#define ION_STREAM_STATE_FLUSHED   1<<5
#define ION_STREAM_STATE_HAS_DATA  1<<6

// runtime stream states
#define ION_STREAM_STATE_CONNECTED 1<<10
#define ION_STREAM_STATE_EOF       1<<11
#define ION_STREAM_STATE_ERROR     1<<12
#define ION_STREAM_STATE_SHUTDOWN  1<<13
#define ION_STREAM_STATE_CLOSED    ION_STREAM_STATE_EOF | ION_STREAM_STATE_ERROR | ION_STREAM_STATE_SHUTDOWN


// mode for line reading (getLine() and awaitLine())
#define ION_STREAM_MODE_TRIM_TOKEN    1
#define ION_STREAM_MODE_WITH_TOKEN    2
#define ION_STREAM_MODE_WITHOUT_TOKEN 4

typedef struct bufferevent bevent;

typedef struct _ion_stream {
    zend_object      std;
    short            state;   // flags ION_STREAM_FLAG_*
    bevent         * buffer;  // input and output bufferevent
    char           * token;   // token for awaitLine
    int              token_length; // token length
    long             length;  // bytes for reading
    long             offset;  // offset for reading next chunk while awaiting
    zval           * read;    // read deferred object
    zval           * flush;   // state deferred object
    zval           * connect; // connection deferred object
    zval           * self;
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

#define ion_stream_new(buffer, state)                  _ion_stream_new(buffer, state, NULL TSRMLS_CC)
#define ion_stream_new_ex(buffer, state, class_entry)  _ion_stream_new(buffer, state, class_entry TSRMLS_CC)
#define ion_stream_zval(zstream, buffer, state, class_entry)    _ion_stream_zval(zstream, buffer, state, NULL TSRMLS_CC)
#define ion_stream_zval_ex(zstream, buffer, state, class_entry) _ion_stream_zval(zstream, buffer, state, class_entry TSRMLS_CC)

int    _ion_stream_zval(zval * zstream, bevent * buffer, short flags, zend_class_entry * cls TSRMLS_DC);
zval * _ion_stream_new(bevent * buffer, short flags, zend_class_entry * cls TSRMLS_DC);

#define CHECK_STREAM(stream)

ion_define_class_entry(ION_Streams);
CLASS_INSTANCE_DTOR(ION_Stream);
CLASS_INSTANCE_CTOR(ION_Stream);

END_EXTERN_C();

#endif //ION_STREAM_H
