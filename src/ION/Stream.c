#include <php.h>
#include <php_network.h>
#include "Stream.h"

ION_DEFINE_CLASS(ION_Stream);

const ion_stream_token empty_stream_token = { NULL, 0, 0, 0, 0, -1 };

#define ion_stream_input_length(stream) evbuffer_get_length( bufferevent_get_input(stream->buffer) )
#define ion_stream_read(stream, size_p) _ion_stream_read(stream, size_p TSRMLS_CC);
#define ion_stream_read_token(stream, data, token) \
    _ion_stream_read_token(stream, data, token TSRMLS_CC)
#define ion_stream_search_token(buffer_p, token_p)  _ion_stream_search_token(buffer_p, token_p TSRMLS_CC)

#define CHECK_STREAM_BUFFER(stream)                          \
    if(stream->buffer == NULL) {                             \
        ThrowRuntime("Stream buffer is not initialized", 1); \
        return;                                              \
    }

#define CHECK_STREAM_STATE(stream)                              \
    if(stream->state & ION_STREAM_STATE_CLOSED) {               \
        if(stream->state & ION_STREAM_STATE_EOF) {              \
            ThrowRuntime("EOF", 1);                             \
        } else if(stream->state & ION_STREAM_STATE_ERROR) {     \
            ThrowRuntime("Stream is corrupted", 1);             \
        } else if(stream->state & ION_STREAM_STATE_SHUTDOWN) {  \
            ThrowRuntime("Stream shutdown", 1);                 \
        }                                                       \
        return;                                                 \
    }

#define CHECK_STREAM(stream)      \
    CHECK_STREAM_BUFFER(stream);  \
    CHECK_STREAM_STATE(stream);

char * _ion_stream_read(ion_stream * stream, size_t * size TSRMLS_DC);
long _ion_stream_read_token(ion_stream * stream, char ** data, ion_stream_token * token TSRMLS_DC);
long _ion_stream_search_token(struct evbuffer * buffer, ion_stream_token * token TSRMLS_DC);

void _ion_stream_input(bevent * bev, void * ctx) {
    ion_stream * stream = (ion_stream *)ctx;
    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
    struct evbuffer * input;
    char * data = NULL;
    size_t read = 0;
    IONF("new data vailable");
    if(stream->read) {
        input = bufferevent_get_input(stream->buffer);
        if(stream->token) { // awaitLine

        } else if(stream->length) { // await()
            if(evbuffer_get_length(input) >= stream->length) {
                read = (size_t)stream->length;
                data = ion_stream_read(stream, &read);
                ion_deferred_done_stringl(stream->read, data, read, 0);
                // freed in the deferred dtor
                ION_CHECK_LOOP_RETURN();
            }
        } // else awaitAll

        if(evbuffer_get_length(input)) {
            stream->state |= ION_STREAM_STATE_HAS_DATA;
        } else {
            stream->state &= ~ION_STREAM_STATE_HAS_DATA;
        }
    } else {
        stream->state |= ION_STREAM_STATE_HAS_DATA;
    }

    if(!stream->read && stream->on_data && (stream->state & ION_STREAM_STATE_HAS_DATA)) {
//        pionCbVoidWith1Arg(stream->on_data, zstream);
    }

    ION_CHECK_LOOP();
}

void _ion_stream_output(bevent *bev, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
    IONF("all data sent");
    if(stream->flush) {
        ion_deferred_done_true(stream->flush);
        stream->flush = NULL;
    }
    stream->state |= ION_STREAM_STATE_FLUSHED;

    ION_CHECK_LOOP();
}

void _ion_stream_notify(bevent *bev, short what, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    char * data = NULL;
    size_t read = 0;

    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
    IONF("stream note");

    if(what & BEV_EVENT_EOF) {
//        PHPDBG("BEV_EVENT_EOF");
        stream->state |= ION_STREAM_STATE_EOF;
        if(stream->read) {
            if(stream->length) {
                // todo fail read-deferred
            } else {
                read = ion_stream_input_length(stream);
                data = ion_stream_read(stream, &read);
                ion_deferred_done_stringl(stream->read, data, read, 0);
            }
        }
    } else if(what & BEV_EVENT_ERROR) {
//        PHPDBG("BEV_EVENT_ERROR");
        stream->state |= ION_STREAM_STATE_ERROR;
    } else if(what & BEV_EVENT_TIMEOUT) {
//        PHPDBG("BEV_EVENT_TIMEOUT");
        if(what & BEV_EVENT_READING) {
            // todo reject read-deferred
        } else {
            // todo reject flush-deferred
        }
    } else if(what & BEV_EVENT_CONNECTED) {
//        PHPDBG("BEV_EVENT_CONNECTED");
        stream->state |= ION_STREAM_STATE_CONNECTED;
        if(stream->connect) {
            // todo done connect-deferred
        }
    } else {
        zend_error(E_WARNING, "Unknown type notification: %d", what);
    }

    ION_CHECK_LOOP();
}

zval * _ion_stream_new(bevent * buffer, short flags, zend_class_entry * cls TSRMLS_DC) {
    zval * zstream;
    ALLOC_INIT_ZVAL(zstream);
    if(ion_stream_zval_ex(zstream, buffer, flags, cls) == FAILURE) {
        return NULL;
    } else {
        return zstream;
    }
}

int _ion_stream_zval(zval * zstream, bevent * buffer, short flags, zend_class_entry * cls TSRMLS_DC) {
    ion_stream * stream;
    if(!cls) {
        cls = CE(ION_Stream);
    }
    object_init_ex(zstream, cls);

    stream = getInstance(zstream);
    stream->buffer = buffer;
    stream->state |= flags;
    bufferevent_setcb(buffer, _ion_stream_input, _ion_stream_output, _ion_stream_notify, (void *) stream);
    if (cls->constructor) {
        return pionCallConstructorWithoutArgs(cls, zstream);
    }
    return SUCCESS;
}

char * _ion_stream_read(ion_stream * stream, size_t * size TSRMLS_DC) {
    size_t incoming_length = ion_stream_input_length(stream);
    char * data;

    if(!incoming_length) {
        data = emalloc(1);
        data[0] = '\0';
        *size = 0;
        return data;
    }
    if(*size > incoming_length) {
        *size = incoming_length;
    }
    data = emalloc(*size + 1);
    *size = bufferevent_read(stream->buffer, data, *size);
    if (*size > 0) {
        data[*size] = '\0';
        return data;
    } else {
        efree(data);
        return NULL;
    }
}

long _ion_stream_read_token(ion_stream * stream, char ** data_out, ion_stream_token * token TSRMLS_DC) {
    size_t size = 0;
    if(token->position == 0) {
        if(token->mode & (ION_STREAM_MODE_WITH_TOKEN | ION_STREAM_MODE_TRIM_TOKEN)) {
            if(evbuffer_drain(bufferevent_get_input(stream->buffer), (size_t)token->token_length) == FAILURE) {
                ThrowRuntime("Failed to drain token", 1);
                return -1;
            }
        }
        if(token->mode == ION_STREAM_MODE_WITH_TOKEN) {
            size = (size_t)token->token_length;
            *data_out  = estrndup(token->token, (unsigned)token->token_length);
        } else {
            size = 0;
            *data_out  = NULL;
        }
    } else {
        if(token->mode == ION_STREAM_MODE_WITH_TOKEN) {
            token->position += token->token_length;
        }

        *data_out = emalloc((size_t)token->position + 1);
        size = bufferevent_read(stream->buffer, *data_out, (size_t) token->position);
        (*data_out)[size] = '\0';
        if(token->mode == ION_STREAM_MODE_TRIM_TOKEN) {
            if(evbuffer_drain(bufferevent_get_input(stream->buffer), (size_t)token->token_length) == FAILURE) {
                efree(*data_out);
                ThrowRuntime("Failed to trim token", 1);
                return -1;
            }
        }
        if (size == 0) {
            efree(*data_out);
        }
    }
    return size;
}

CLASS_INSTANCE_DTOR(ION_Stream) {
    ion_stream * stream = getInstanceObject(ion_stream *);
    if(stream->flush) {
        ion_deferred_free(stream->flush);
        zval_ptr_dtor(&stream->flush);
    }
    if(stream->read) {
        ion_deferred_free(stream->read);
        zval_ptr_dtor(&stream->read);
        if(stream->token) {
            efree(stream->token);
            stream->token = NULL;
        }
        stream->read = NULL;
    }
    if(stream->connect) {
        ion_deferred_free(stream->connect);
        zval_ptr_dtor(&stream->connect);
    }
    if(stream->buffer) {
        bufferevent_disable(stream->buffer, EV_READ | EV_WRITE);
        bufferevent_free(stream->buffer);
    }
    efree(stream);
}

CLASS_INSTANCE_CTOR(ION_Stream) {
    ion_stream * stream = emalloc(sizeof(ion_stream));
    memset(stream, 0, sizeof(ion_stream));
    TSRMLS_SET_CTX(stream->thread_ctx);
    RETURN_INSTANCE(ION_Stream, stream);
}


/** public static function ION\Stream::resource(resource $resource) : self */
CLASS_METHOD(ION_Stream, resource) {
    zval *zfd;
    int fd = -1, fd2;
    int flags = STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE;
    short state = 0;
    bevent *buffer = NULL;

    PARSE_ARGS("r", &zfd);

    php_stream *stream_resource;
    if (ZEND_FETCH_RESOURCE_NO_RETURN(stream_resource, php_stream *, &zfd, -1, NULL, php_file_le_stream())) {
        if(php_stream_cast(stream_resource, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL | PHP_STREAM_AS_SOCKETD, (void *) &fd, 0) == SUCCESS) {
            state = ION_STREAM_STATE_SOCKET;
        } else if (php_stream_cast(stream_resource, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, 0) == FAILURE) {
            ThrowInvalidArgument("stream argument must be either valid PHP stream");
            return;
        }
    }

    fd2 = dup(fd);
    if (fd2 == -1) {
        ThrowRuntimeEx(errno, "Failed to duplicate fd: %s", strerror(errno));
        return;
    }

    buffer = bufferevent_socket_new(ION(base), fd2, flags);
    if(NULL == buffer) {
        ThrowRuntime("Failed to create Stream: buffer corrupted", -1);
        return;
    }
    ion_stream_zval_ex(return_value, buffer, state, EG(called_scope));
}

METHOD_ARGS_BEGIN(ION_Stream, resource, 1)
     METHOD_ARG(resource, 0)
METHOD_ARGS_END()

/** public static function ION\Stream::pair() : self[] */
CLASS_METHOD(ION_Stream, pair) {
    int flags = STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE;

    bevent * pair[2];
    zval *one;
    zval *two;

    if(bufferevent_pair_new(ION(base), flags, pair) == FAILURE) {
        ThrowRuntime("Failed to create pair", 1);
        return;
    }

    one = ion_stream_new_ex(pair[0], ION_STREAM_STATE_SOCKET | ION_STREAM_STATE_PAIR, EG(called_scope));
    two = ion_stream_new_ex(pair[1], ION_STREAM_STATE_SOCKET | ION_STREAM_STATE_PAIR, EG(called_scope));

    array_init(return_value);
    add_next_index_zval(return_value, one);
    add_next_index_zval(return_value, two);


    if (EG(called_scope)->constructor) {
        if(pionCallConstructor(EG(called_scope), one, 0, NULL TSRMLS_CC) == FAILURE) {
            return;
        }
        if(pionCallConstructor(EG(called_scope), two, 0, NULL TSRMLS_CC) == FAILURE) {
            return;
        }
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, pair)


/** public static function ION\Stream::socket(string $host) : self */
CLASS_METHOD(ION_Stream, socket) {
    char * host;
    char * port_digits;
    char * hostname;
    long host_len     = 0;
    long port         = 0;
    bevent * buffer   = NULL;
    struct sockaddr;
    PARSE_ARGS("s", &host, &host_len);

    buffer = bufferevent_socket_new(ION(base), -1, STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);

    if(buffer == NULL) {
        ThrowRuntime("Error creating the socket", 1);
        return;
    }

    // split host to hostname and port
    long hostname_len = host_len;
    for(; hostname_len > 0; --hostname_len) {
        if(host[ hostname_len ] == ':') {
            break;
        }
    }
    hostname = emalloc((size_t)hostname_len + 1);
    strncpy(hostname, host, hostname_len);
    hostname[hostname_len] = '\0';
    port_digits = emalloc((size_t)host_len - hostname_len + 2);
    strncpy(port_digits, host + hostname_len + 1, host_len - hostname_len + 1);
    port_digits[host_len - hostname_len + 1] = '\0';
    port = strtol(port_digits, NULL, 10);
    efree(port_digits);

    if(bufferevent_socket_connect_hostname(buffer, ION(evdns), AF_UNSPEC, hostname, (int)port) == FAILURE) {
        efree(hostname);
        ThrowRuntime("Failed to connect", 1);
        return;
    }
    efree(hostname);

    ion_stream_zval_ex(return_value, buffer, ION_STREAM_STATE_SOCKET, EG(called_scope));
}

METHOD_ARGS_BEGIN(ION_Stream, socket, 1)
    METHOD_ARG(host, 0)
METHOD_ARGS_END()

/** private function ION\Stream::_input() : void */
CLASS_METHOD(ION_Stream, _input) {

}

METHOD_WITHOUT_ARGS(ION_Stream, _input)

/** private function ION\Stream::_output() : void */
CLASS_METHOD(ION_Stream, _output) {

}

METHOD_WITHOUT_ARGS(ION_Stream, _output)

/** private function ION\Stream::_notify() : void */
CLASS_METHOD(ION_Stream, _notify) {

}

METHOD_WITHOUT_ARGS(ION_Stream, _notify)

/** public function ION\Stream::enable() : self */
CLASS_METHOD(ION_Stream, enable) {
    ion_stream * stream = getThisInstance();

    CHECK_STREAM_BUFFER(stream);
    if(bufferevent_enable(stream->buffer, EV_READ | EV_WRITE)) {
        ThrowRuntime("Failed to enable stream", 1);
        return;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream, enable)

/** public function ION\Stream::disable() : self */
CLASS_METHOD(ION_Stream, disable) {
    ion_stream * stream = getThisInstance();

    CHECK_STREAM_BUFFER(stream);
    if(bufferevent_disable(stream->buffer, EV_READ | EV_WRITE)) {
        ThrowRuntime("Failed to disable stream", 1);
        return;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream, disable)


/** public function ION\Stream::awaitConnection() : Deferred */
CLASS_METHOD(ION_Stream, awaitConnection) {

}

METHOD_WITHOUT_ARGS(ION_Stream, awaitConnection)

/** public function ION\Stream::setTimeouts(double $read_timeout, double $write_timeout) : self */
CLASS_METHOD(ION_Stream, setTimeouts) {
    ion_stream * stream = getThisInstance();
    double read_timeout = 0.0, write_timeout = 0.0;
    struct timeval read_tv = { 0, 0 }, write_tv = {0, 0};

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("dd", &read_timeout, &write_timeout);
    if(read_timeout < 0 || write_timeout < 0) {
        ThrowRuntime("timeout sould be unsigned", 1);
        return;
    }
    SET_TIMEVAL(read_tv, read_timeout);
    SET_TIMEVAL(write_tv, write_timeout);
    bufferevent_set_timeouts(stream->buffer, &read_tv, &write_tv);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Stream, setTimeouts, 2)
    METHOD_ARG(read_timeout, 0)
    METHOD_ARG(write_timeout, 0)
METHOD_ARGS_END()


/** public function ION\Stream::setPriority(int $priority) : self */
CLASS_METHOD(ION_Stream, setPriority) {
    int prio = 0;
    ion_stream * stream = getThisInstance();

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("l", &prio);
    if(bufferevent_priority_set(stream->buffer, prio) == FAILURE) {
        ThrowRuntime("bufferevent_priority_set failed", 1);
        return;
    }
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Stream, setPriority, 1)
    METHOD_ARG(priority, 0)
METHOD_ARGS_END()

/** public function ION\Stream::write(string $data) : self */
CLASS_METHOD(ION_Stream, write) {
    char *data;
    int data_len = 0;
    ion_stream * stream = getThisInstance();

    CHECK_STREAM(stream);
    PARSE_ARGS("s", &data, &data_len);

    if(!data_len) {
        RETURN_THIS();
    }

    if(bufferevent_write(stream->buffer, (const void *)data, (size_t)data_len)) {
        ThrowRuntime("Failed to write data", 1);
        return;
    }

    bufferevent_flush(stream->buffer, EV_WRITE, BEV_NORMAL);

    if(evbuffer_get_length(bufferevent_get_output(stream->buffer))) {
        stream->state &= ~ION_STREAM_STATE_FLUSHED;
    }
    RETURN_THIS();


}

METHOD_ARGS_BEGIN(ION_Stream, write, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()

/** public function ION\Stream::sendFile(resource $fd, int $offset = 0, int $limit = -1) : self */
CLASS_METHOD(ION_Stream, sendFile) {
    zval * zfd          = NULL;
    long offset         = 0;
    long length         = -1;
    int fd              = -1;
    int fd2;
    ion_stream * stream = getThisInstance();

    CHECK_STREAM(stream);
    PARSE_ARGS("r|ll", &zfd, &offset, &length);

    php_stream * stream_resource;
    if (ZEND_FETCH_RESOURCE_NO_RETURN(stream_resource, php_stream *, &zfd, -1, NULL, php_file_le_stream())) {
        if (php_stream_cast(stream_resource, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, REPORT_ERRORS) == FAILURE) {
            ThrowInvalidArgument("stream argument must be either valid PHP stream");
            return;
        }
    }

    errno = 0;
    fd2 = dup(fd);
    if(fd2 == -1) {
        ThrowRuntimeEx(errno, "Failed to duplicate fd for add_file: %s", strerror(errno));
        return;
    }

    if(evbuffer_add_file(
            bufferevent_get_output(stream->buffer),
            fd2,
            (ev_off_t)offset,
            (ev_off_t)length
    )) {
        ThrowRuntime("Failed to send file", 1);
        return;
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Stream, sendFile, 1)
    METHOD_ARG(fd, 0)
    METHOD_ARG(offset, 0)
    METHOD_ARG(limit, 0)
METHOD_ARGS_END()

void _deferred_stream_dtor(void *object, zval * zdeferred TSRMLS_DC) {
    ion_stream * stream = (ion_stream *) object;
    if(stream->flush) {
        zval_ptr_dtor(&stream->flush);
        stream->flush = NULL;
    }
}

/** public function ION\Stream::state() : ION\Deferred */
CLASS_METHOD(ION_Stream, flush) {
    ion_stream * stream = getThisInstance();

    CHECK_STREAM(stream);
    if(stream->flush) {
        RETURN_ZVAL(stream->flush, 1, 0);
    }

    stream->flush = ion_deferred_new_ex(NULL);
    zval_addref_p(stream->flush);
    ion_deferred_store(stream->flush, stream, _deferred_stream_dtor);
    if(stream->state & ION_STREAM_STATE_FLUSHED) {
        // TODO state delay
    }
    RETURN_ZVAL(stream->flush, 1, 0);
}

METHOD_WITHOUT_ARGS(ION_Stream, flush)

long _ion_stream_search_token(struct evbuffer * buffer, ion_stream_token * token TSRMLS_DC) {
    struct evbuffer_ptr ptr_end;
    struct evbuffer_ptr ptr_start;
    struct evbuffer_ptr ptr_result;
    size_t current_size = evbuffer_get_length(buffer);
    size_t end = (size_t)token->offset + (size_t)token->length - 1;
    size_t length = (size_t)token->length;
    if(current_size == 0) {
        token->position = -1;
        return SUCCESS;
    }
    if(token->offset >= current_size || token->token_length > current_size) {
        token->position = -1;
        return SUCCESS;
    }
    if(end >= current_size - 1) { // libevent bug? if <end> is last element - evbuffer_search_range can't find token
        length = 0;
    }

    if(evbuffer_ptr_set(buffer, &ptr_start, (size_t)token->offset, EVBUFFER_PTR_SET) == FAILURE) {
        return FAILURE;
    }
    if(length) {
        if(evbuffer_ptr_set(buffer, &ptr_end, end, EVBUFFER_PTR_SET) == FAILURE) {
            return FAILURE;
        }
        ptr_result = evbuffer_search_range(buffer, token->token, (size_t)token->token_length, &ptr_start, &ptr_end);
    } else {
        ptr_result = evbuffer_search(buffer, token->token, (size_t)token->token_length, &ptr_start);
    }
    token->offset = current_size - token->token_length + 1;
    token->position = (long)ptr_result.pos;
    return SUCCESS;
}


/** public function ION\Stream::search(string $token, int $offset = 0, int $length = 0) : int|bool */
CLASS_METHOD(ION_Stream, search) {
    ion_stream * stream = getThisInstance();
    ion_stream_token token = empty_stream_token;
    struct evbuffer * buffer;

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("s|ll", &token.token, &token.token_length, &token.offset, &token.length);

    if(!token.token_length) {
        ThrowInvalidArgument("Empty token string");
        return;
    }

    buffer = bufferevent_get_input(stream->buffer);
    if(ion_stream_search_token(buffer, &token) == FAILURE) {
        ThrowRuntime("Failed to get internal buffer pointer for token_length/offset", -1);
        return;
    }

    RETURN_LONG(token.position);
}

METHOD_ARGS_BEGIN(ION_Stream, search, 1)
    METHOD_ARG(token, 0)
    METHOD_ARG(length, 0)
    METHOD_ARG(offset, 0)
METHOD_ARGS_END()

/** public function ION\Stream::getSize(int $type = self::INPUT) : string|bool */
CLASS_METHOD(ION_Stream, getSize) {
    ion_stream * stream = getThisInstance();
    long type = EV_READ;
    struct evbuffer *buffer;

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("|l", &type);

    if(type == EV_READ) {
        buffer = bufferevent_get_input(stream->buffer);
    } else if(type == EV_WRITE) {
        buffer = bufferevent_get_output(stream->buffer);
    } else {
        ThrowInvalidArgument("Invalid buffer identify");
        return;
    }

    RETURN_LONG(evbuffer_get_length(buffer));
}

METHOD_ARGS_BEGIN(ION_Stream, getSize, 0)
    METHOD_ARG(type, 0)
METHOD_ARGS_END()


/** public function ION\Stream::get(int $bytes) : string */
CLASS_METHOD(ION_Stream, get) {
    ion_stream * stream = getThisInstance();
    size_t length = 0;
    char * data = NULL;

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("|l", &length);

    if(!length) {
        RETURN_EMPTY_STRING();
    }

    data = ion_stream_read(stream, &length);
    if(data == NULL) {
        ThrowRuntime("Stream is unreachable", -1);
        return;
    }
    RETURN_STRINGL(data, length, 0);
}

METHOD_ARGS_BEGIN(ION_Stream, get, 0)
    METHOD_ARG(bytes, 0)
METHOD_ARGS_END()

/** public function ION\Stream::getAll() : string */
CLASS_METHOD(ION_Stream, getAll) {
    ion_stream * stream = getThisInstance();
    size_t length;
    char * data = NULL;

    CHECK_STREAM_BUFFER(stream);
    length = (long)ion_stream_input_length(stream);
    data = ion_stream_read(stream, &length);
    if(data == NULL) {
        ThrowRuntime("Stream is unreachable", -1);
        return;
    }
    RETURN_STRINGL(data, length, 0);
}

METHOD_WITHOUT_ARGS(ION_Stream, getAll);

/** public function ION\Stream::getLine(string $token, $mode = self::MODE_TRIM_TOKEN, $max_length = 0) : string|bool */
CLASS_METHOD(ION_Stream, getLine) {
    ion_stream * stream = getThisInstance();
    ion_stream_token token = empty_stream_token;
    char * data;
    long size;

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("s|ll", &token.token, &token.token_length, &token.mode, &token.length);
    if(token.token_length == 0) {
        RETURN_FALSE;
    }

    if(ion_stream_search_token(bufferevent_get_input(stream->buffer), &token) == FAILURE) {
        ThrowRuntime("Failed to get internal buffer pointer for token_length/offset", -1);
        return;
    }

    if(token.position == -1) {
        RETURN_FALSE;
    } else {
        size = ion_stream_read_token(stream, &data, &token);
        if(size == -1) {
            return;
        } else if(size == 0) {
            RETVAL_EMPTY_STRING();
        } else {
            RETURN_STRINGL(data, size, 0);
        }
    }
}

METHOD_ARGS_BEGIN(ION_Stream, getLine, 1)
    METHOD_ARG(token, 0)
    METHOD_ARG(mode, 0)
    METHOD_ARG(max_length, 0)
METHOD_ARGS_END()

void _deferred_stream_await_dtor(void *object, zval *zdeferred TSRMLS_DC) {
    ion_stream * stream = (ion_stream *) object;
    bufferevent_setwatermark(stream->buffer, EV_READ, 0, 0);
    if(stream->read) {
        zval_ptr_dtor(&stream->read);
        if(stream->token) {
            efree(stream->token->token);
            efree(stream->token);
        }
        stream->read = NULL;
        stream->token = NULL;
    }
    zval_ptr_dtor(&zdeferred);
}

/** public function ION\Stream::await(int $bytes) : ION\Deferred */
CLASS_METHOD(ION_Stream, await) {
    ion_stream * stream = getThisInstance();
    size_t length = 0;
    size_t current = 0;
    char * data = NULL;
    zval * zdeferred = NULL;

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("l", &length);
    if(stream->read) {
        ThrowLogic("Stream already reading", -1);
        return;
    }

    current = ion_stream_input_length(stream);
    zdeferred = ion_deferred_new_ex(NULL);
    if(current >= length) {
        data = ion_stream_read(stream, &length);
        if(data == NULL) {
            ThrowRuntime("Stream is unreachable", -1);
            return;
        }
        ion_deferred_done_stringl(zdeferred, data, length, 0);
        RETURN_ZVAL(zdeferred, 1, 0);
    } else {
        ion_deferred_store(zdeferred, stream, _deferred_stream_await_dtor);
        stream->read = zdeferred;
        stream->length = length;
        zval_add_ref(&zdeferred);
        bufferevent_setwatermark(stream->buffer, EV_READ, length - current, 0);
        RETURN_ZVAL_FAST(zdeferred);
    }
}

METHOD_ARGS_BEGIN(ION_Stream, await, 1)
    METHOD_ARG(bytes, 0)
METHOD_ARGS_END()

/** public function ION\Stream::awaitLine(string $token, $mode = self::MODE_TRIM_TOKEN, $max_length = 0) : ION\Deferred */
CLASS_METHOD(ION_Stream, awaitLine) {
    ion_stream * stream = getThisInstance();
    ion_stream_token token = empty_stream_token;
    zval * zdeferred = NULL;
    char * data;
    long size;

    CHECK_STREAM_BUFFER(stream);
    PARSE_ARGS("s|ll", &token.token, &token.token_length, &token.mode, &token.length);
    if(token.token_length == 0) {
        RETURN_FALSE;
    }


    if(ion_stream_search_token(bufferevent_get_input(stream->buffer), &token) == FAILURE) {
        ThrowRuntime("Failed to get internal buffer pointer for token_length/offset", -1);
        return;
    }

    zdeferred = ion_deferred_new_ex(NULL);

    if(token.position == -1) { // not found
        stream->token = emalloc(sizeof(ion_stream_token));
        memset(stream->token, 0, sizeof(ion_stream_token));
        stream->token->token = estrndup(token.token, (unsigned)token.token_length);
        stream->token->token_length = token.token_length;
        stream->token->mode = token.mode;
        stream->token->offset = token.offset;
        stream->token->length = token.length;
        stream->token->position = -1;
        RETURN_ZVAL_FAST(zdeferred);
    } else { // found
        size = ion_stream_read_token(stream, &data, &token);
        if(size == -1) {
            ion_deferred_free(zdeferred);
            return;
        } else if(size == 0) {
            ion_deferred_done_empty_string(zdeferred);
        } else {
            ion_deferred_done_stringl(zdeferred, data, size, 0);
        }
        RETURN_ZVAL(zdeferred, 1, 0);
    }
}

METHOD_ARGS_BEGIN(ION_Stream, awaitLine, 1)
    METHOD_ARG(token, 0)
    METHOD_ARG(mode, 0)
    METHOD_ARG(max_length, 0)
METHOD_ARGS_END()

/** public function ION\Stream::awaitAll(int $bytes) : ION\Deferred */
CLASS_METHOD(ION_Stream, awaitAll) {
    ion_stream * stream = getThisInstance();
    zval * zdeferred;
    CHECK_STREAM_BUFFER(stream);
    if(stream->read) {
        ThrowLogic("Stream already reading", -1);
        return;
    }
    zdeferred = ion_deferred_new_ex(NULL);
    if(stream->state & ION_STREAM_STATE_EOF) {
        ion_deferred_done_empty_string(zdeferred);
        RETURN_EMPTY_STRING();
        RETURN_ZVAL(zdeferred, 1, 0);
    } else {
        ion_deferred_store(zdeferred, stream, _deferred_stream_await_dtor);
        stream->read = zdeferred;
        stream->length = 0;
        zval_add_ref(&zdeferred);
        RETURN_ZVAL_FAST(zdeferred);
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, awaitAll)

/** public function ION\Stream::shutdown() : self */
CLASS_METHOD(ION_Stream, shutdown) {
    ion_stream * stream = getThisInstance();
    CHECK_STREAM_BUFFER(stream);
    if(stream->state & ION_STREAM_STATE_CLOSED) {
        RETURN_THIS();
    }
    // todo
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream, shutdown)

/** public function ION\Stream::onData(callable $callback) : self */
CLASS_METHOD(ION_Stream, onData) {

}

METHOD_ARGS_BEGIN(ION_Stream, onData, 1)
    METHOD_ARG(callback, 0)
METHOD_ARGS_END()

/** public function ION\Stream::onClose(callable $callback) : self */
CLASS_METHOD(ION_Stream, onClose) {

}

METHOD_ARGS_BEGIN(ION_Stream, onClose, 1)
    METHOD_ARG(callback, 0)
METHOD_ARGS_END()

/** public function ION\Stream::ensureSSL(ION\SSL $ssl) : self */
CLASS_METHOD(ION_Stream, ensureSSL) {

}

METHOD_ARGS_BEGIN(ION_Stream, ensureSSL, 1)
    METHOD_ARG(ssl, 0)
METHOD_ARGS_END()

/** public function ION\Stream::getRemotePeer() : string */
CLASS_METHOD(ION_Stream, getRemotePeer) {

}

METHOD_WITHOUT_ARGS(ION_Stream, getRemotePeer)

/** public function ION\Stream::getLocalPeer() : string */
CLASS_METHOD(ION_Stream, getLocalPeer) {

}

METHOD_WITHOUT_ARGS(ION_Stream, getLocalPeer)


/** public function ION\Stream::__destruct() : void */
CLASS_METHOD(ION_Stream, __debugInfo) {

}

METHOD_WITHOUT_ARGS(ION_Stream, __debugInfo)

/** public function ION\Stream::__destruct() : void */
CLASS_METHOD(ION_Stream, __destruct) {
    ion_stream * stream = getThisInstance();
    if(stream->flush) {
        ion_deferred_reject(stream->flush, "The stream shutdown by the destructor");
        // stream->flush freed in cancel function
    }
    if(stream->read) {
        ion_deferred_reject(stream->read, "The stream shutdown by the destructor");
        // stream->read freed in cancel function
    }
    if(stream->connect) {
        ion_deferred_reject(stream->connect, "The stream shutdown by the destructor");
        // stream->flush freed in cancel function
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, __destruct)

/** public function ION\Stream::__toString() : string */
CLASS_METHOD(ION_Stream, __toString) {
    ion_stream * stream = getThisInstance();
    if(stream->state & ION_STREAM_STATE_SOCKET) {
        RETURN_STRING("stream:socket()", 1);
    } else if(stream->state & ION_STREAM_STATE_PAIR) {
        RETURN_STRING("stream:pair", 1);
    } else {
        RETURN_STRING("stream:pipe()", 1);
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, __toString)

#ifdef ION_DEBUG
/** public function ION\Stream::appendToInput(string $data) : self */
CLASS_METHOD(ION_Stream, appendToInput) {
    char *data;
    int data_len = 0;
    ion_stream * stream = getThisInstance();
    struct evbuffer * input;

    CHECK_STREAM(stream);
    PARSE_ARGS("s", &data, &data_len);

    if(!data_len) {
        RETURN_THIS();
    }

    input = bufferevent_get_input(stream->buffer);

    if(evbuffer_add(input, (const void *)data, (size_t)data_len)) {
        ThrowRuntime("Failed to append data to input", 1);
        return;
    }
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Stream, appendToInput, 1)
    METHOD_ARG(data, 0)
METHOD_ARGS_END()
#endif

CLASS_METHODS_START(ION_Stream)
    METHOD(ION_Stream, resource,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, pair,            ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, socket,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#ifdef ION_DEBUG
    METHOD(ION_Stream, _input,          ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, _output,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, _notify,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, appendToInput,   ZEND_ACC_PUBLIC)
#else
    METHOD(ION_Stream, _incoming,       ZEND_ACC_PRIVATE)
    METHOD(ION_Stream, _empty,          ZEND_ACC_PRIVATE)
    METHOD(ION_Stream, _notify,         ZEND_ACC_PRIVATE)
#endif
    METHOD(ION_Stream, enable,          ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, disable,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, awaitConnection, ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, setTimeouts,     ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, setPriority,     ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, write,           ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, sendFile,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, flush,           ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, search,          ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getSize,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, get,             ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getAll,          ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getLine,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, await,           ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, awaitAll,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, awaitLine,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, shutdown,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, onData,          ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, onClose,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, ensureSSL,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getRemotePeer,   ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getLocalPeer,    ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, __destruct,      ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, __toString,      ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, __debugInfo,     ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Stream) {
    PION_REGISTER_CLASS(ION_Stream, "ION\\Stream");
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_TRIM_TOKEN", ION_STREAM_MODE_TRIM_TOKEN);
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_WITH_TOKEN", ION_STREAM_MODE_WITH_TOKEN);
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_WITHOUT_TOKEN", ION_STREAM_MODE_WITHOUT_TOKEN);

    PION_CLASS_CONST_LONG(ION_Stream, "INPUT",  EV_READ);
    PION_CLASS_CONST_LONG(ION_Stream, "OUTPUT", EV_WRITE);
    PION_CLASS_CONST_LONG(ION_Stream, "BOTH",   EV_WRITE | EV_READ);
    return SUCCESS;
}


PHP_RINIT_FUNCTION(ION_Stream) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Stream) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Stream) {
    return SUCCESS;
}