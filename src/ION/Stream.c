#include <php.h>
#include "Stream.h"

zend_bool has_socket_module = 0;

void _ion_stream_read(bevent *bev, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
}

void _ion_stream_write(bevent *bev, void *ctx) {
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
    bufferevent_setcb(stream->buffer, _ion_stream_read, _ion_stream_write, _ion_stream_notify, (void *)stream);
}

CLASS_INSTANCE_DTOR(ION_Stream) {
    ion_stream * stream = getInstanceObject(ion_stream *);
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
        if(pionCallConstructor(EG(called_scope), return_value, 0, NULL) == FAILURE) {
            zval_ptr_dtor(&return_value);
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

    if(bufferevent_pair_new(ION(base), flags, pair) == FAILURE) {
        ThrowRuntime("Failed to create pair", 1);
        return;
    }

    zval *one;
    zval *two;

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
        if(pionCallConstructor(EG(called_scope), one, 0, NULL) == FAILURE) {
            return;
        }
        if(pionCallConstructor(EG(called_scope), two, 0, NULL) == FAILURE) {
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
        if(pionCallConstructor(EG(called_scope), return_value, 0, NULL) == FAILURE) {
            zval_ptr_dtor(&return_value);
            return;
        }
    }

}

METHOD_ARGS_BEGIN(ION_Stream, socket, 1)
    METHOD_ARG(hostname, 0)
    METHOD_ARG(port, 0)
METHOD_ARGS_END()


/** public function ION\Stream::enable() : self */
CLASS_METHOD(ION_Stream, enable) {
    ion_stream * stream = getThisInstance();
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

/** public function ION\Stream::flush() : Deferred */
CLASS_METHOD(ION_Stream, flush) {
    ion_stream * stream = getThisInstance();

    stream->flush = deferredNewInternal(NULL);
    zval_addref_p(stream->flush);
    deferredStore(stream->flush, stream, _deferred_stream_dtor);

    RETURN_ZVAL(stream->flush, 1, 0);
}

METHOD_WITHOUT_ARGS(ION_Stream, flush)


CLASS_METHODS_START(ION_Stream)
    METHOD(ION_Stream, resource,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, pair,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, socket,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, enable,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, disable,      ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, setTimeouts,  ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, setPriority,  ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, write,        ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, sendFile,     ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, flush,        ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Stream) {
    PION_REGISTER_CLASS(ION_Stream, "ION\\Stream");
//    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_RejectException, Exception, "ION\\Deferred\\RejectException", NULL);
//    REGISTER_VOID_EXTENDED_CLASS(ION_Deferred_TimeoutException, ION_Deferred_RejectException, "ION\\Deferred\\TimeoutException", NULL);
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