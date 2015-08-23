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
    TSRMLS_SET_CTX(stream->thread_ctx)
    RETURN_INSTANCE(ION_Stream, stream);
}


/** public static function ION\Stream::resource(resource $resource) : static */
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

/** public static function ION\Stream::pair() : static[] */
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


/** public static function ION\Stream::connect(string $hostname, int $port = 0) : static */
CLASS_METHOD(ION_Stream, connect) {
    char * host;
    char * port_digits;
    char * hostname;
    long host_len     = 0;
    long port         = 0;
    bevent * buffer   = NULL;
    PARSE_ARGS("s", &host, &host_len);



    buffer = bufferevent_socket_new(ION(base), -1, STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);

    if(buffer == NULL) {
        ThrowRuntime("Error creating the socket", 1);
        return;
    }

    // split hostname and port from host
    long hostname_len = host_len;
    for(hostname_len; hostname_len > 0; --hostname_len) {
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
    ion_stream_set_buffer(getInstance(return_value), buffer);

    if (EG(called_scope)->constructor) {
        if(pionCallConstructor(EG(called_scope), return_value, 0, NULL) == FAILURE) {
            zval_ptr_dtor(&return_value);
            return;
        }
    }

}

METHOD_ARGS_BEGIN(ION_Stream, connect, 1)
    METHOD_ARG(hostname, 0)
    METHOD_ARG(port, 0)
METHOD_ARGS_END()

CLASS_METHODS_START(ION_Stream)
    METHOD(ION_Stream, resource, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, pair,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, connect,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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