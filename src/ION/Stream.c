#include <php.h>
#include "Stream.h"

zend_bool has_socket_module = 0;

void _ion_stream_data(bevent *bev, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
}

void _ion_stream_empty(bevent *bev, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
    if(stream->flush) {
        zval *result;
        ALLOC_LONG_ZVAL(result, 0);
        deferredResolve(stream->flush, result);
        zval_ptr_dtor(&result);
        zval_ptr_dtor(&stream->flush);
        stream->flush = NULL;
    }
    stream->flags |= ION_STREAM_FLAG_FLUSHED;
}

void _ion_stream_notify(bevent *bev, short what, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
}

void ion_stream_set_buffer(ion_stream * stream, bevent * buffer) {
    stream->buffer = buffer;
    bufferevent_setcb(stream->buffer, _ion_stream_data, _ion_stream_empty, _ion_stream_notify, (void *)stream);
}

CLASS_INSTANCE_DTOR(ION_Stream) {
    ion_stream * stream = getInstanceObject(ion_stream *);
    if(stream->flush) {
        deferredFree(stream->flush);
        zval_ptr_dtor(&stream->flush);
    }
    if(stream->read) {
        deferredFree(stream->read);
        zval_ptr_dtor(&stream->read);
        if(stream->token) {
            efree(stream->token);
        }
    }
    if(stream->connect) {
        deferredFree(stream->connect);
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
    bevent * buffer = NULL;

    PARSE_ARGS("r", &zfd);

    php_stream * stream_resource;
    if (ZEND_FETCH_RESOURCE_NO_RETURN(stream_resource, php_stream *, &zfd, -1, NULL, php_file_le_stream())) {
        if (php_stream_cast(stream_resource, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, REPORT_ERRORS) == FAILURE) {
            ThrowInvalidArgument("stream argument must be either valid PHP stream");
            return;
        }
    }

    fd2 = dup(fd);
    if(fd2 == -1) {
        ThrowRuntimeEx(errno, "Failed to duplicate fd: %s", strerror(errno));
        return;
    }

    object_init_ex(return_value, EG(called_scope));
    ion_stream * stream = getInstance(return_value);
    buffer = bufferevent_socket_new(ION(base), fd2, flags);
    if(NULL == buffer) {
        ThrowRuntime("Failed to create Stream: buffer corrupted", -1);
        return;
    }

    ion_stream_set_buffer(stream, buffer);

    if (EG(called_scope)->constructor) {
        if(pionCallConstructor(EG(called_scope), return_value, 0, NULL TSRMLS_CC) == FAILURE) {
            return;
        }
    }
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
//    ion_stream * stream_one;
//    ion_stream * stream_two;

    if(bufferevent_pair_new(ION(base), flags, pair) == FAILURE) {
        ThrowRuntime("Failed to create pair", 1);
        return;
    }

    ALLOC_INIT_ZVAL(one);
    ALLOC_INIT_ZVAL(two);

    object_init_ex(one, EG(called_scope));
    object_init_ex(two, EG(called_scope));

    ion_stream_set_buffer(getInstance(one), pair[0]);
    ion_stream_set_buffer(getInstance(two), pair[1]);

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


/** public static function ION\Stream::socket(string $hostname, int $port = 0) : self */
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

    object_init_ex(return_value, EG(called_scope));
    ion_stream * stream = getInstance(return_value);
    ion_stream_set_buffer(getInstance(return_value), buffer);
    stream->flags |= ION_STREAM_FLAG_SOCKET;


    if (EG(called_scope)->constructor) {
        if(pionCallConstructor(EG(called_scope), return_value, 0, NULL TSRMLS_CC) == FAILURE) {
            return;
        }
    }

}

METHOD_ARGS_BEGIN(ION_Stream, socket, 1)
    METHOD_ARG(host, 0)
METHOD_ARGS_END()


/** public function ION\Stream::enable() : self */
CLASS_METHOD(ION_Stream, enable) {
    ion_stream * stream = getThisInstance();

    CHECK_STREAM(stream);
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

    CHECK_STREAM(stream);
    if(bufferevent_disable(stream->buffer, EV_READ | EV_WRITE)) {
        ThrowRuntime("Failed to disable stream", 1);
        return;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream, disable)


/** public function ION\Stream::setTimeouts(double $read_timeout, double $write_timeout) : self */
CLASS_METHOD(ION_Stream, setTimeouts) {
    ion_stream * stream = getThisInstance();
    double read_timeout = 0.0, write_timeout = 0.0;
    struct timeval read_tv = { 0, 0 }, write_tv = {0, 0};

    CHECK_STREAM(stream);
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

    CHECK_STREAM(stream);
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
        stream->flags &= ~ION_STREAM_FLAG_FLUSHED;
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

/** public function ION\Stream::flush() : ION\Deferred */
CLASS_METHOD(ION_Stream, flush) {
    ion_stream * stream = getThisInstance();

    CHECK_STREAM(stream);
    if(stream->flush) {
        RETURN_ZVAL(stream->flush, 1, 0);
    }

    stream->flush = deferredNewInternal(NULL);
    zval_addref_p(stream->flush);
    deferredStore(stream->flush, stream, _deferred_stream_dtor);
    if(stream->flags & ION_STREAM_FLAG_FLUSHED) {
        // TODO flush delay
    }
    RETURN_ZVAL(stream->flush, 1, 0);
}

METHOD_WITHOUT_ARGS(ION_Stream, flush)

int _ion_stream_search(struct evbuffer_ptr * result, struct evbuffer * buffer, char * token, size_t token_len, size_t offset, size_t length) {
    struct evbuffer_ptr * ptr_length = NULL;
    struct evbuffer_ptr * ptr_offset = NULL;
    struct evbuffer_ptr result_ptr;
    if(offset) {
        if(evbuffer_ptr_set(buffer, ptr_offset, (size_t)offset, EVBUFFER_PTR_SET)) {
            return FAILURE;
        }
    }
    if(length) {
        if(evbuffer_ptr_set(buffer, ptr_length, (size_t)length, EVBUFFER_PTR_SET)) {
            return FAILURE;
        }
        *result = evbuffer_search_range(buffer, token, (size_t)token_len, ptr_offset, ptr_length);
    } else {
        *result = evbuffer_search(buffer, token, (size_t)token_len, ptr_offset);
    }
    return SUCCESS;
}

#define ion_stream_search(result, buffer, token, token_len, offset, length) \
    _ion_stream_search(result, buffer, token, (size_t) token_len, (size_t) offset, (size_t) length)

/** public function ION\Stream::search(string $token, int $length = 0, int $offset = 0) : int|bool */
CLASS_METHOD(ION_Stream, search) {
    ion_stream * stream = getThisInstance();
    char * token;
    long token_len      = 0;
    long length         = 0;
    long offset         = 0;
    struct evbuffer * buffer;
    struct evbuffer_ptr result;

    CHECK_STREAM(stream);
    PARSE_ARGS("s|ll", &token, &token_len, &length, &offset);

    if(!token_len) {
        ThrowInvalidArgument("Empty token string");
        return;
    }

    buffer = bufferevent_get_input(stream->buffer);

    if(ion_stream_search(&result, buffer, token, token_len, offset, length) == FAILURE) {
        ThrowRuntime("Failed to get internal buffer pointer for length/offset", -1);
        return;
    }

    if(result.pos < 0) {
        RETURN_FALSE;
    } else {
        RETURN_LONG(result.pos);
    }

}

METHOD_ARGS_BEGIN(ION_Stream, search, 1)
    METHOD_ARG(token, 0)
    METHOD_ARG(length, 0)
    METHOD_ARG(offset, 0)
METHOD_ARGS_END()

/** public function ION\Stream::getSize(int $type = ION::READ) : string|bool */
CLASS_METHOD(ION_Stream, getSize) {
    ion_stream * stream = getThisInstance();
    long type = EV_READ;
    struct evbuffer *buffer;

    CHECK_STREAM(stream);
    PARSE_ARGS("|l", &type);

    if(type == EV_READ) {
        buffer = bufferevent_get_input(stream->buffer);
    } else if(type == EV_WRITE) {
        buffer = bufferevent_get_output(stream->buffer);
    } else {
        ThrowRuntime("Failed to retrieve buffer", 1);
        return;
    }

    RETURN_LONG(evbuffer_get_length(buffer));
}

METHOD_ARGS_BEGIN(ION_Stream, getSize, 0)
    METHOD_ARG(type, 0)
METHOD_ARGS_END()

char * _ion_stream_read(ion_stream * stream, size_t size) {
    char * data;
    size_t read;
    data = emalloc(size + 1);
    long incoming_length;
    incoming_length = (long)evbuffer_get_length( bufferevent_get_input(stream->buffer) );
    if(size > incoming_length) {
        size = (size_t) incoming_length;
    }
    read = bufferevent_read(stream->buffer, data, size);
    if (read > 0) {
        data[read] = '\0';
        return data;
    } else {
        efree(data);
        return NULL;
    }
}

#define ion_stream_read(stream, size) _ion_stream_read(stream, (size_t) size)

/** public function ION\Stream::get(int $bytes) : string */
CLASS_METHOD(ION_Stream, get) {
    ion_stream * stream = getThisInstance();
    long length = -1;
    char * data;

    CHECK_STREAM(stream);
    PARSE_ARGS("|l", &length);

    if(length < 0) {
        ThrowInvalidArgument("bytes < 0");
        return;
    }
    if(!length) {
        RETURN_EMPTY_STRING();
    }

    data = ion_stream_read(stream, length);
    if(data == NULL) {
        ThrowRuntime("Failed to reading", -1);
        return;
    }
    RETURN_STRING(data, 0);
}

METHOD_ARGS_BEGIN(ION_Stream, get, 0)
    METHOD_ARG(bytes, 0)
METHOD_ARGS_END()

/** public function ION\Stream::getAll() : string */
CLASS_METHOD(ION_Stream, getAll) {
    ion_stream * stream = getThisInstance();
    long length;
    char * data;

    CHECK_STREAM(stream);
    length = (long)evbuffer_get_length( bufferevent_get_input(stream->buffer) );
    data = ion_stream_read(stream, length);
    if(data == NULL) {
        ThrowRuntime("Failed to reading", -1);
        return;
    }
    RETURN_STRING(data, 0);
}

METHOD_WITHOUT_ARGS(ION_Stream, getAll);

/** public function ION\Stream::getLine(string $token, $mode = self::WITHOUT_TOKEN, $max_length = 8192) : string|bool */
CLASS_METHOD(ION_Stream, getLine) {
    ion_stream * stream = getThisInstance();
    char * data;
    char * token;
    long token_len      = 0;
    long mode           = 0;
    long max_length     = 0;
    struct evbuffer * buffer;
    struct evbuffer_ptr result;
    size_t len;
    size_t ret;

    CHECK_STREAM(stream);
    PARSE_ARGS("s|ll", &token, &token_len, &mode, &max_length);

    buffer = bufferevent_get_input(stream->buffer);

    if(ion_stream_search(&result, buffer, token, token_len, 0, max_length) == FAILURE) {
        ThrowRuntime("Failed to get internal buffer pointer for length/offset", -1);
        return;
    }

    if(result.pos < 0) {
        RETURN_FALSE;
    } else {
        len = (size_t)result.pos;
        if(len == 0) {
            if(mode == ION_STREAN_LN_TRIM_WITH_TOKEN) {
                RETURN_STRINGL(token, token_len, 0)
            } else {
                RETURN_EMPTY_STRING();
            }
        }

        if(mode == ION_STREAN_LN_TRIM_WITH_TOKEN) {
            len += token_len;
        }

        data = safe_emalloc((size_t)len + 1, sizeof(char), 1);
        ret = bufferevent_read(stream->buffer, data, len);
        if(mode == ION_STREAN_LN_TRIM_TOKEN) {
            if(evbuffer_drain(buffer, (size_t)token_len) == FAILURE) {
                efree(data);
                ThrowRuntime("Failed to drain token", 1);
            }
        }
        if (ret > 0) {
            RETURN_STRINGL(data, len, 0);
        } else {
            efree(data);
            RETURN_EMPTY_STRING();
        }
    }
}

METHOD_ARGS_BEGIN(ION_Stream, getLine, 1)
    METHOD_ARG(token, 0)
    METHOD_ARG(mode, 0)
    METHOD_ARG(max_length, 0)
METHOD_ARGS_END()

void _deferred_stream_read_dtor(void *object, zval * zdeferred TSRMLS_DC) {
    ion_stream * stream = (ion_stream *) object;
    if(stream->read) {
        zval_ptr_dtor(&stream->read);
        stream->read = NULL;
    }
}

/** public function ION\Stream::await(int $bytes) : ION\Deferred */
CLASS_METHOD(ION_Stream, await) {
    ion_stream * stream = getThisInstance();
    long length = 0;
    size_t current = 0;
    char * data;

    CHECK_STREAM(stream);
    PARSE_ARGS("l", &length);
    if(stream->read) {
        ThrowLogic("Stream already reading", -1);
        return;
    }

    current = (long)evbuffer_get_length( bufferevent_get_input(stream->buffer) );
    stream->read = deferredNewInternal(NULL);
    zval_addref_p(stream->read);
    deferredStore(stream->read, stream, _deferred_stream_dtor);
    if(current >= length) {
        // TODO read delay
    }
    RETURN_ZVAL(stream->read, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Stream, await, 1)
    METHOD_ARG(bytes, 0)
METHOD_ARGS_END()

/** public function ION\Stream::awaitLine(string $token, $mode = self::WITHOUT_TOKEN, $max_length = 8192) : ION\Deferred */
CLASS_METHOD(ION_Stream, awaitLine) {

}

METHOD_ARGS_BEGIN(ION_Stream, awaitLine, 1)
    METHOD_ARG(token, 0)
    METHOD_ARG(mode, 0)
    METHOD_ARG(max_length, 0)
METHOD_ARGS_END()

/** public function ION\Stream::awaitAll(int $bytes) : ION\Deferred */
CLASS_METHOD(ION_Stream, awaitAll) {

}

METHOD_WITHOUT_ARGS(ION_Stream, awaitAll)

/** public function ION\Stream::shutdown() : self */
CLASS_METHOD(ION_Stream, shutdown) {

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
CLASS_METHOD(ION_Stream, __destruct) {
    ion_stream * stream = getThisInstance();
    if(stream->flush) {
        deferredReject(stream->flush, "The stream interrupted by the destructor");
        zval_ptr_dtor(&stream->flush);
        stream->flush = NULL;
    }
    if(stream->read) {
        deferredReject(stream->read, "The stream interrupted by the destructor");
        zval_ptr_dtor(&stream->read);
        stream->read = NULL;
        if(stream->token) {
            efree(stream->token);
            stream->token = NULL;
        }
    }
    if(stream->connect) {
        deferredReject(stream->connect, "The stream interrupted by the destructor");
        zval_ptr_dtor(&stream->connect);
        stream->connect = NULL;
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, __destruct)

/** public function ION\Stream::__toString() : string */
CLASS_METHOD(ION_Stream, __toString) {
    ion_stream * stream = getThisInstance();
    if(stream->flags & ION_STREAM_FLAG_SOCKET) {
        RETURN_STRING("Socket()", 1);
    } else if(stream->flags & ION_STREAM_FLAG_PAIR) {
        RETURN_STRING("pipe", 1);
    } else {
        RETURN_STRING("pipe()", 1);
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, __toString)

CLASS_METHODS_START(ION_Stream)
    METHOD(ION_Stream, resource,      ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, pair,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, socket,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, enable,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, disable,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, setTimeouts,   ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, setPriority,   ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, write,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, sendFile,      ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, flush,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, search,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getSize,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, get,           ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getAll,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getLine,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, await,         ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, awaitAll,      ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, awaitLine,     ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, shutdown,      ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, onData,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, onClose,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, ensureSSL,     ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getRemotePeer, ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getLocalPeer,  ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, __destruct,    ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, __toString,    ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Stream) {
    PION_REGISTER_CLASS(ION_Stream, "ION\\Stream");
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_TRIM_TOKEN", ION_STREAN_LN_TRIM_TOKEN);
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_WITH_TOKEN", ION_STREAN_LN_TRIM_WITH_TOKEN);
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_WITHOUT_TOKEN", ION_STREAN_LN_TRIM_WITHOUT_TOKEN);
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