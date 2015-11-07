
#include "../pion.h"
#include <php_network.h>
#include <fcntl.h>
#include <ext/standard/url.h>
//#include "Stream.h"

#ifndef O_NOATIME
# define O_NOATIME 0
#endif

zend_class_entry     * ion_ce_ION_Stream;
zend_object_handlers   ion_oh_ION_Stream;
zend_class_entry     * ion_ce_ION_Stream_RuntimeException;
zend_object_handlers   ion_oh_ION_Stream_RuntimeException;

const ion_stream_token empty_stream_token = { NULL, 0, 0, ION_STREAM_MODE_TRIM_TOKEN, -1 };


zend_object * ion_stream_init(zend_class_entry * ce) {
    ion_stream * istream = ecalloc(1, sizeof(ion_stream));
    RETURN_INSTANCE(ION_Stream, istream);
}

void ion_stream_free(zend_object * stream) {
    ion_stream * istream = get_object_instance(stream, ion_stream);
    if(istream->flush) {
        zend_object_release(istream->flush);
        istream->flush = NULL;
    }
    if(istream->read) {
        zend_object_release(istream->read);
        if(istream->token) {
            if(istream->token->token) {
                zend_string_release(istream->token->token);
            }
            efree(istream->token);
            istream->token = NULL;
        }
        istream->read = NULL;
    }
    if(istream->connect) {
        zend_object_release(istream->connect);
        istream->connect = NULL;
    }
    if(istream->buffer) {
        if(istream->state & ION_STREAM_STATE_ENABLED) {
            bufferevent_disable(istream->buffer, EV_READ | EV_WRITE);
        }
        bufferevent_free(istream->buffer);
    }
    if(istream->name_self) {
        zend_string_release(istream->name_self);
    }
    if(istream->name_remote) {
        zend_string_release(istream->name_remote);
    }
}

zend_string * ion_stream_get_name_self(zend_object * stream_obj) {
    ion_stream * stream = get_object_instance(stream_obj, ion_stream);
    int type   = 0;
    evutil_socket_t socket;
    if(stream->name_self == NULL) {
        socket = bufferevent_getfd(stream->buffer);
        if(socket == -1) {
            return NULL;
        } else {
            type = pion_net_sock_name(socket, PION_NET_NAME_LOCAL, &stream->name_self);
            if(type == PION_NET_NAME_IPV6) {
                stream->state |= ION_STREAM_NAME_IPV6;
            } else if(type == PION_NET_NAME_UNIX) {
                stream->state |= ION_STREAM_NAME_UNIX;
            } else if(type == PION_NET_NAME_UNKNOWN || type == FAILURE) {
                return NULL;
            } else {
                stream->state |= ION_STREAM_NAME_IPV4;
            }
        }
    }
    return zend_string_copy(stream->name_self);
}

zend_string * ion_stream_get_name_remote(zend_object * stream_obj) {
    ion_stream * stream = get_object_instance(stream_obj, ion_stream);
    int type   = 0;
    evutil_socket_t socket;
    if(stream->name_remote == NULL) {
        socket = bufferevent_getfd(stream->buffer);
        if(socket == -1) {
            return NULL;
        } else {
            type = pion_net_sock_name(socket, PION_NET_NAME_REMOTE, &stream->name_remote);
            if(type == PION_NET_NAME_UNKNOWN || type == FAILURE) {
                return NULL;
            }
        }
    }
    return zend_string_copy(stream->name_remote);
}

void _ion_stream_input(bevent * bev, void * ctx) {

    ion_stream      * stream = get_object_instance(ctx, ion_stream);
    struct evbuffer * input;
    zend_string     * data = NULL;
//    size_t            read = 0;
//    IONF("new data available");
    if(stream->read) {
        input = bufferevent_get_input(stream->buffer);
        if(stream->token) { // awaitLine
            if(ion_stream_search_token(bufferevent_get_input(stream->buffer), stream->token) == FAILURE) {
//                ion_deferred_exception(stream->read, ION_Stream_RuntimeException(), "Failed to get internal buffer pointer for token_length/offset", -1);
            } else if(stream->token->position != -1) { // found
                data = ion_stream_read_token(stream, stream->token);
                if(!data) {
                    ion_promisor_exception_eg(
                            stream->read,
                            ion_class_entry(ION_Stream_RuntimeException),
                            "Stream corrupted: failed to read token from buffer", 0
                    );
                } else {
//                    ion_deferred_done_stringl(stream->read, data, read, 0);
                }
            } else if(stream->token->flags & ION_STREAM_TOKEN_LIMIT) {
//                ion_deferred_done_false(stream->read);
            }
        } else if(stream->length) { // await()

            if(evbuffer_get_length(input) >= stream->length) {
                data = ion_stream_read(stream, stream->length);
                bufferevent_setwatermark(stream->buffer, EV_READ, 0, stream->input_size);
                ion_promisor_done_string(stream->read, data, 0);
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

    if(stream->read == NULL && stream->on_data != NULL && (stream->state & ION_STREAM_STATE_HAS_DATA)) {
//        pionCbVoidWith1Arg(stream->on_data, zstream);
    }

    ION_CHECK_LOOP();
}

void _ion_stream_output(bevent * bev, void *ctx) {
    ion_stream *stream = get_object_instance(ctx, ion_stream);

    IONF("all data sent");
    if(stream->flush) {
//        ion_deferred_done_true(stream->flush);
        stream->flush = NULL;
    }
    stream->state |= ION_STREAM_STATE_FLUSHED;
    if(stream->state & ION_STREAM_STATE_CLOSE_ON_FLUSH) {
        ion_stream_close_fd(stream);
    }

    ION_CHECK_LOOP();
}

void _ion_stream_notify(bevent * bev, short what, void *ctx) {
    ion_stream *stream = (ion_stream *)ctx;
    zend_string * data = NULL;
    size_t read = 0;

    TSRMLS_FETCH_FROM_CTX(stream->thread_ctx);
    IONF("stream note");

    if(what & BEV_EVENT_EOF) {
        stream->state |= ION_STREAM_STATE_EOF;
        if(stream->read) {
            if(stream->token) {
//                ion_deferred_done_false(stream->read);
            } else {
                read = ion_stream_input_length(stream);
                data = ion_stream_read(stream, read);
//                ion_deferred_done_stringl(stream->read, data, read, 0);
            }
        }
        if(stream->shutdown) {
//            ion_deferred_done(stream->closing, stream->self);
        }
    } else if(what & BEV_EVENT_ERROR) {
        stream->state |= ION_STREAM_STATE_ERROR;
    } else if(what & BEV_EVENT_TIMEOUT) {
        if(what & BEV_EVENT_READING) {
            // todo reject read-deferred
        } else {
            // todo reject flush-deferred
        }
    } else if(what & BEV_EVENT_CONNECTED) {
//        PHPDBG("connected");
        stream->state |= ION_STREAM_STATE_CONNECTED;
        if(stream->connect) {
//            ion_deferred_done(stream->connect, stream->self);
        }
    } else {
        zend_error(E_WARNING, "Unknown type notification: %d", what);
    }

    ION_CHECK_LOOP();
}


zend_object * ion_stream_new_ex(bevent * buffer, int flags, zend_class_entry * cls) {
    ion_stream * stream;
    zval         zstream;
    if(!cls) {
        cls = ion_class_entry(ION_Stream);
    }
    object_init_ex(&zstream, cls);
    stream = get_instance(&zstream, ion_stream);

    stream->buffer = buffer;
    stream->state |= flags;
    if(flags & ION_STREAM_DIRECT_INCOMING) {
        stream->state |= ION_STREAM_STATE_FLUSHED;
    } else {
        if(ion_stream_output_length(stream) == 0) {
            stream->state |= ION_STREAM_STATE_FLUSHED;
        }
        if(ion_stream_input_length(stream)) {
            stream->state |= ION_STREAM_STATE_HAS_DATA;
        }
    }
    bufferevent_setcb(buffer, _ion_stream_input, _ion_stream_output, _ion_stream_notify, (void *) stream);
    if (cls->constructor) {
        if(pion_call_constructor_without_args(cls, Z_OBJ(zstream)) == FAILURE) {
            zval_ptr_dtor(&zstream);
            return NULL;
        }
    }
    return Z_OBJ(zstream);
}


zend_string * ion_stream_read(ion_stream * stream, size_t size) {
    size_t incoming_length = ion_stream_input_length(stream);
    zend_string * data;

    if(!incoming_length) {
        return zend_string_init("", 0, 0);
    }
    if(size > incoming_length) {
        size = incoming_length;
    }

    data = zend_string_alloc(size, 0);
    ZSTR_LEN(data) = bufferevent_read(stream->buffer, ZSTR_VAL(data), size);
    if (ZSTR_LEN(data) > 0) {
        ZSTR_VAL(data)[ZSTR_LEN(data)] = '\0';
        return data;
    } else {
        zend_string_free(data);
        return NULL;
    }
}

zend_string * ion_stream_read_token(ion_stream * stream, ion_stream_token * token) {
    if(token->position == 0) {
        if(token->flags & (ION_STREAM_MODE_WITH_TOKEN | ION_STREAM_MODE_TRIM_TOKEN)) {
            if(evbuffer_drain(bufferevent_get_input(stream->buffer), token->token->len) == FAILURE) {
                zend_throw_exception(ion_class_entry(ION_RuntimeException), "Failed to drain token", 0);
                return NULL;
            }
        }
        if(token->flags & ION_STREAM_MODE_WITH_TOKEN) {
            return zend_string_dup(token->token, 0);
        } else {
            return NULL;
        }
    } else {
        zend_string * data;
        if(token->flags & ION_STREAM_MODE_WITH_TOKEN) {
            token->position += token->token->len;
        }

        data = zend_string_alloc((size_t)token->position + 1, 0);
        ZSTR_LEN(data) = bufferevent_read(stream->buffer, ZSTR_VAL(data), (size_t)token->position);
        ZSTR_VAL(data)[ZSTR_LEN(data)] = '\0';
        if(token->flags & ION_STREAM_MODE_TRIM_TOKEN) {
            if(evbuffer_drain(bufferevent_get_input(stream->buffer), token->token->len) == FAILURE) {
                zend_string_release(data);
                zend_throw_exception(ion_class_entry(ION_RuntimeException), "Failed to drain token", 0);
                return NULL;
            }
        }
        if (ZSTR_LEN(data) == 0) {
            zend_string_release(data);
            return zend_string_init("", 0, 0);
        }
        return data;
    }
}

int ion_stream_close_fd(ion_stream * stream) {
    evutil_socket_t socket;
    bufferevent_disable(stream->buffer, EV_READ | EV_WRITE);

    stream->state |= ION_STREAM_STATE_SHUTDOWN;
    socket = bufferevent_getfd(stream->buffer);
    if(socket == -1) {
        return SUCCESS;
    }
    if(stream->state & ION_STREAM_STATE_SOCKET) {
        evutil_closesocket(socket);
    } else if(socket > 2) { // skip stdin, stdout, stderr
        close(socket);
    }
    bufferevent_setfd(stream->buffer, -1);
    return SUCCESS;
}

/** public static function ION\Stream::resource(resource $resource) : self */
CLASS_METHOD(ION_Stream, resource) {
    zval        * zfd = NULL;
    int           fd = -1;
    int           fd2;
    int           flags = STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE;
    int           state = 0;
    bevent      * buffer = NULL;
    zend_object * stream = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(zfd)
    ZEND_PARSE_PARAMETERS_END();
//    PARSE_ARGS("r", &zfd);

    php_stream *stream_resource;
    php_stream_from_zval_no_verify(stream_resource, zfd);
    if (stream_resource) {
        if(php_stream_cast(stream_resource, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL | PHP_STREAM_AS_SOCKETD, (void *) &fd, 0) == SUCCESS) {
            state = ION_STREAM_STATE_SOCKET;
        } else if (php_stream_cast(stream_resource, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, 0) == FAILURE) {
            zend_throw_exception(ion_class_entry(InvalidArgumentException), "Argument must be either valid PHP stream resource", 0);
            return;
        }
    }

    fd2 = dup(fd);
    if (fd2 == -1) {
        zend_throw_exception_ex(ion_class_entry(ION_Stream_RuntimeException), errno, "Failed to duplicate fd: %s", strerror(errno));
        return;
    }

    buffer = bufferevent_socket_new(ION(base), fd2, flags);
    if(NULL == buffer) {
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Failed to create Stream: buffer corrupted", 0);
        return;
    }
    if(bufferevent_enable(buffer, EV_READ | EV_WRITE)) {
        bufferevent_free(buffer);
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Failed to enable stream", 0);
        return;
    }
    stream = ion_stream_new_ex(buffer, state | ION_STREAM_STATE_ENABLED, zend_get_called_scope(execute_data));
    if(!stream) {
        // todo check EG(exception)
        return;
    }
    RETURN_OBJ(stream);
}

METHOD_ARGS_BEGIN(ION_Stream, resource, 1)
    METHOD_ARG_RESOURCE(resource, 0)
METHOD_ARGS_END()

/** public static function ION\Stream::pair() : self[] */
CLASS_METHOD(ION_Stream, pair) {
    int flags = STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE;
    int state = ION_STREAM_STATE_SOCKET | ION_STREAM_STATE_PAIR | ION_STREAM_STATE_ENABLED | ION_STREAM_STATE_CONNECTED;

    bevent * pair[2];
    zend_object * one;
    zend_object * two;
    zval          zstream_one;
    zval          zstream_two;
    zend_class_entry * ce = zend_get_called_scope(execute_data);

    if(bufferevent_pair_new(ION(base), flags, pair) == FAILURE) {
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Failed to create pair", 0);
        return;
    }
    if(bufferevent_enable(pair[0], EV_READ | EV_WRITE) == FAILURE ||
       bufferevent_enable(pair[1], EV_READ | EV_WRITE) == FAILURE) {
        bufferevent_free(pair[0]);
        bufferevent_free(pair[1]);
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Failed to enable stream", 0);
        return;
    }

    one = ion_stream_new_ex(pair[0], state, ce);
    two = ion_stream_new_ex(pair[1], state, ce);
    if(!one || !two) { // constructor failed
        if(one) {
            zend_object_release(one);
        }
        if(two) {
            zend_object_release(two);
        }
        // todo check EG(exception)
        return;
    }

    ZVAL_OBJ(&zstream_one, one);
    ZVAL_OBJ(&zstream_two, two);

    array_init(return_value);
    add_next_index_zval(return_value, &zstream_one);
    add_next_index_zval(return_value, &zstream_two);
}

METHOD_WITHOUT_ARGS(ION_Stream, pair)


/** public static function ION\Stream::socket(string $host) : self */
CLASS_METHOD(ION_Stream, socket) {
    zend_string * host = NULL;
    php_url * resource;
    bevent * buffer   = NULL;
    zend_object * stream = NULL;

//    int host_len     = 0;
//    long port         = 0;
//    char * port_digits;
//    char * hostname;
    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(host)
    ZEND_PARSE_PARAMETERS_END();
//    PARSE_ARGS("s", &host, &host_len);
    resource = php_url_parse_ex(host->val, host->len);
    if (resource == NULL) {
        zend_throw_exception(ion_class_entry(InvalidArgumentException), "Invalid socket name", 0);
        return;
    }
    buffer = bufferevent_socket_new(ION(base), -1, STREAM_BUFFER_DEFAULT_FLAGS | BEV_OPT_CLOSE_ON_FREE);

    if(buffer == NULL) {
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Error creating the socket", 0);
        php_url_free(resource);
        return;
    }

    if(resource->host) { // ip:port, [ipv6]:port, hostname:port
        if(bufferevent_socket_connect_hostname(buffer, /*ION(dns)->evdns*/ NULL, AF_UNSPEC, resource->host, resource->port ? (int)resource->port : 0) == FAILURE) {
            php_url_free(resource);
            bufferevent_free(buffer);
            zend_throw_exception_ex(ion_class_entry(ION_Stream_RuntimeException), 0, "Failed to connect to %s: %s", host->val, strerror(errno));
            return;
        }
    } else if(resource->path) { // unix domain socket: /path/to/socket.sock
        // todo
    } else {
        php_url_free(resource);
        bufferevent_free(buffer);
        zend_throw_exception(ion_class_entry(InvalidArgumentException), "Invalid socket name", 0);
        return;
    }
    php_url_free(resource);

    // split host to hostname and port
//    long hostname_len = host_len;
//    for(; hostname_len > 0; --hostname_len) {
//        if(host[ hostname_len ] == ':') {
//            break;
//        }
//    }
//    hostname = emalloc((size_t)hostname_len + 1);
//    strncpy(hostname, host, hostname_len);
//    hostname[hostname_len] = '\0';
//    port_digits = emalloc((size_t)host_len - hostname_len + 2);
//    strncpy(port_digits, host + hostname_len + 1, host_len - hostname_len + 1);
//    port_digits[host_len - hostname_len + 1] = '\0';
//    port = strtol(port_digits, NULL, 10);
//    efree(port_digits);
//
//    if(bufferevent_socket_connect_hostname(buffer, /*ION(dns)->evdns*/ NULL, AF_UNSPEC, hostname, (int)port) == FAILURE) {
//        efree(hostname);
//        ThrowRuntime("Failed to connect ", 1);
//        return;
//    }
//    efree(hostname);
    if(bufferevent_enable(buffer, EV_READ | EV_WRITE)) {
        bufferevent_free(buffer);
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Failed to enable stream", 0);
        return;
    }

    stream = ion_stream_new_ex(buffer, ION_STREAM_STATE_SOCKET | ION_STREAM_STATE_ENABLED, zend_get_called_scope(execute_data));
    if(!stream) {
        // todo check EG(exception)
        return;
    }
    RETURN_OBJ(stream);
}

METHOD_ARGS_BEGIN(ION_Stream, socket, 1)
    METHOD_ARG_STRING(host, 0)
METHOD_ARGS_END()
//
///** private function ION\Stream::_input() : void */
//CLASS_METHOD(ION_Stream, _input) {
//
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, _input)
//
///** private function ION\Stream::_output() : void */
//CLASS_METHOD(ION_Stream, _output) {
//
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, _output)
//
///** private function ION\Stream::_notify() : void */
//CLASS_METHOD(ION_Stream, _notify) {
//
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, _notify)
//
/** public function ION\Stream::enable() : self */
CLASS_METHOD(ION_Stream, enable) {
    ion_stream * stream = get_this_instance(ion_stream);

    CHECK_STREAM_BUFFER(stream);
    if(bufferevent_enable(stream->buffer, EV_READ | EV_WRITE)) {
        zend_throw_exception(ion_class_entry(ION_Stream_RuntimeException), "Failed to enable stream", 0);
        return;
    }
    stream->state |= ION_STREAM_STATE_ENABLED;
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Stream, enable)

/** public function ION\Stream::disable() : self */
//CLASS_METHOD(ION_Stream, disable) {
//    ion_stream * stream = getThisInstance();
//
//    CHECK_STREAM_BUFFER(stream);
//    if(bufferevent_disable(stream->buffer, EV_READ | EV_WRITE) == SUCCESS) {
//        stream->state &= ~ION_STREAM_STATE_ENABLED;
//    }
//    RETURN_THIS();
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, disable)
//
//void _deferred_stream_connect_dtor(void * object, zval * zdeferred TSRMLS_DC) {
//    ion_stream * stream = (ion_stream *) object;
//    if(stream->connect) {
//        zval_ptr_dtor(&stream->connect);
//        stream->connect = NULL;
//    }
//}
//
///** public function ION\Stream::awaitConnection() : Deferred */
//CLASS_METHOD(ION_Stream, awaitConnection) {
//    ion_stream * stream = getThisInstance();
//    zval * zdeferred;
//    CHECK_STREAM_BUFFER(stream);
//    zdeferred = ion_deferred_new_ex(NULL);
//    if(stream->state & ION_STREAM_STATE_CONNECTED) {
//        ion_deferred_done(zdeferred, getThis());
//    } else {
//        ion_deferred_store(zdeferred, stream, _deferred_stream_connect_dtor);
//        stream->connect = zdeferred;
//    }
//    RETURN_ZVAL(zdeferred, 1, 0);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, awaitConnection)
//
///** public function ION\Stream::setTimeouts(double $read_timeout, double $write_timeout) : self */
//CLASS_METHOD(ION_Stream, setTimeouts) {
//    ion_stream * stream = getThisInstance();
//    double read_timeout = 0.0, write_timeout = 0.0;
//    struct timeval read_tv = { 0, 0 }, write_tv = {0, 0};
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("dd", &read_timeout, &write_timeout);
//    if(read_timeout < 0 || write_timeout < 0) {
//        ThrowRuntime("timeout sould be unsigned", 1);
//        return;
//    }
//    SET_TIMEVAL(read_tv, read_timeout);
//    SET_TIMEVAL(write_tv, write_timeout);
//    bufferevent_set_timeouts(stream->buffer, &read_tv, &write_tv);
//    RETURN_THIS();
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, setTimeouts, 2)
//    METHOD_ARG_DOUBLE(read_timeout, 0)
//    METHOD_ARG_DOUBLE(write_timeout, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::setPriority(int $priority) : self */
//CLASS_METHOD(ION_Stream, setPriority) {
//    int prio = 0;
//    ion_stream * stream = getThisInstance();
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("l", &prio);
//    if(bufferevent_priority_set(stream->buffer, prio) == FAILURE) {
//        ThrowRuntime("bufferevent_priority_set failed", 1);
//        return;
//    }
//    RETURN_THIS();
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, setPriority, 1)
//    METHOD_ARG_LONG(priority, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::setInputSize(int $bytes) : self */
//CLASS_METHOD(ION_Stream, setInputSize) {
//    long bytes = 0;
//    ion_stream * stream = getThisInstance();
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("l", &bytes);
//    if(bytes < 0) {
//        pion_throw(ion_get_class(ION_InvalidArgumentException), "The number of bytes cannot be negative", -1);
//        return;
//    }
//    stream->input_size = (size_t)bytes;
//    bufferevent_setwatermark(stream->buffer, EV_READ, stream->length, (stream->input_size >= stream->length) ? stream->input_size : stream->length);
//    RETURN_THIS();
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, setInputSize, 1)
//    METHOD_ARG_LONG(bytes, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::write(string $data) : self */
//CLASS_METHOD(ION_Stream, write) {
//    char *data;
//    int data_len = 0;
//    ion_stream * stream = getThisInstance();
//
//    CHECK_STREAM(stream);
//    PARSE_ARGS("s", &data, &data_len);
//
//    if(!data_len) {
//        RETURN_THIS();
//    }
//
//    if(bufferevent_write(stream->buffer, (const void *)data, (size_t)data_len)) {
//        ThrowRuntime("Failed to write data", 1);
//        return;
//    }
//
////    bufferevent_flush(stream->buffer, EV_WRITE, BEV_NORMAL);
////
////    if(evbuffer_get_length(bufferevent_get_output(stream->buffer))) {
////        stream->state &= ~ION_STREAM_STATE_FLUSHED;
////    }
//    RETURN_THIS();
//
//
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, write, 1)
//    METHOD_ARG_STRING(data, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::sendFile(resource $fd, int $offset = 0, int $limit = -1) : self */
//CLASS_METHOD(ION_Stream, sendFile) {
//    char * filename     = NULL;
//    long   filename_len = 0;
//    long   offset       = 0;
//    long   length       = -1;
//    int    fd;
//    ion_stream * stream = getThisInstance();
//
//    CHECK_STREAM(stream);
//    PARSE_ARGS("s|ll", &filename, &filename_len, &offset, &length);
//
//    errno = 0;
//    fd = open(filename, O_RDONLY | O_CLOEXEC | O_NONBLOCK | O_NOATIME);
//    if(fd == -1) {
//        ThrowRuntimeEx(errno, "failed to open file: %s", strerror(errno));
//        return;
//    }
//
//    if(evbuffer_add_file(
//            bufferevent_get_output(stream->buffer),
//            fd,
//            (ev_off_t)offset,
//            (ev_off_t)length
//    )) {
//        close(fd);
//        ThrowRuntime("Failed to send file", 1);
//        return;
//    }
//
//    RETURN_THIS();
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, sendFile, 1)
//    METHOD_ARG_RESOURCE(fd, 0)
//    METHOD_ARG_LONG(offset, 0)
//    METHOD_ARG_LONG(limit, 0)
//METHOD_ARGS_END()
//
//void _deferred_stream_flush_dtor(void * object, zval * zdeferred TSRMLS_DC) {
//    ion_stream * stream = (ion_stream *) object;
//    if(stream->flush) {
//        zval_ptr_dtor(&stream->flush);
//        stream->flush = NULL;
//    }
//}
//
///** public function ION\Stream::state() : ION\Deferred */
//CLASS_METHOD(ION_Stream, flush) {
//    ion_stream * stream = getThisInstance();
//    zval * zdeferred = NULL;
//
//    CHECK_STREAM(stream);
//    if(stream->flush) {
//        RETURN_ZVAL(stream->flush, 1, 0);
//    }
//
//    CHECK_STREAM_BUFFER(stream);
//    zdeferred = ion_deferred_new_ex(NULL);
//    if(stream->state & ION_STREAM_STATE_FLUSHED) {
//        ion_deferred_done_true(zdeferred);
//    } else {
//        ion_deferred_store(zdeferred, stream, _deferred_stream_flush_dtor);
//        stream->flush = zdeferred;
//    }
//    RETURN_ZVAL(zdeferred, 1, 0);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, flush)
//
//long _ion_stream_search_token(struct evbuffer * buffer, ion_stream_token * token TSRMLS_DC) {
//    struct evbuffer_ptr ptr_end;
//    struct evbuffer_ptr ptr_start;
//    struct evbuffer_ptr ptr_result;
//    size_t current_size = evbuffer_get_length(buffer);
//    size_t end = (size_t)token->offset + (size_t)token->length - 1;
//    size_t length = (size_t)token->length;
//    if(current_size == 0) {
//        token->position = -1;
//        return SUCCESS;
//    }
//    if(token->offset >= current_size || token->token_length > current_size) {
//        token->position = -1;
//        return SUCCESS;
//    }
//    if(end >= current_size - 1) { // libevent bug? if <end> is last element - evbuffer_search_range can't find token
//        length = 0;
//    }
//
//    if(evbuffer_ptr_set(buffer, &ptr_start, (size_t)token->offset, EVBUFFER_PTR_SET) == FAILURE) {
//        return FAILURE;
//    }
//    if(length) {
//        if(evbuffer_ptr_set(buffer, &ptr_end, end, EVBUFFER_PTR_SET) == FAILURE) {
//            return FAILURE;
//        }
//        ptr_result = evbuffer_search_range(buffer, token->token, (size_t)token->token_length, &ptr_start, &ptr_end);
//    } else {
//        ptr_result = evbuffer_search(buffer, token->token, (size_t)token->token_length, &ptr_start);
//    }
//    if(token->length > 0 && current_size >= token->length) {
//        token->flags |= ION_STREAM_TOKEN_LIMIT;
//    }
//    token->offset = current_size - token->token_length + 1;
//    token->position = (long)ptr_result.pos;
//    return SUCCESS;
//}
//
//
///** public function ION\Stream::search(string $token, int $offset = 0, int $length = 0) : int|bool */
//CLASS_METHOD(ION_Stream, search) {
//    ion_stream * stream = getThisInstance();
//    ion_stream_token token = empty_stream_token;
//    struct evbuffer * buffer;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("s|ll", &token.token, &token.token_length, &token.offset, &token.length);
//
//    if(!token.token_length) {
//        ThrowInvalidArgument("Empty token string");
//        return;
//    }
//
//    buffer = bufferevent_get_input(stream->buffer);
//    if(ion_stream_search_token(buffer, &token) == FAILURE) {
//        ThrowRuntime("Failed to get internal buffer pointer for token_length/offset", -1);
//        return;
//    }
//
//    RETURN_LONG(token.position);
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, search, 1)
//    METHOD_ARG_STRING(token, 0)
//    METHOD_ARG_LONG(length, 0)
//    METHOD_ARG_LONG(offset, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::getSize(int $type = self::INPUT) : string|bool */
//CLASS_METHOD(ION_Stream, getSize) {
//    ion_stream * stream = getThisInstance();
//    long type = EV_READ;
//    struct evbuffer *buffer;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("|l", &type);
//
//    if(type == EV_READ) {
//        buffer = bufferevent_get_input(stream->buffer);
//    } else if(type == EV_WRITE) {
//        buffer = bufferevent_get_output(stream->buffer);
//    } else {
//        ThrowInvalidArgument("Invalid buffer identify");
//        return;
//    }
//
//    RETURN_LONG(evbuffer_get_length(buffer));
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, getSize, 0)
//    METHOD_ARG(type, 0)
//METHOD_ARGS_END()
//
//
///** public function ION\Stream::get(int $bytes) : string */
//CLASS_METHOD(ION_Stream, get) {
//    ion_stream * stream = getThisInstance();
//    size_t length = 0;
//    char * data = NULL;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("|l", &length);
//
//    if(!length) {
//        RETURN_EMPTY_STRING();
//    }
//
//    data = ion_stream_read(stream, &length);
//    if(data == NULL) {
//        ThrowRuntime("Stream is unreachable", -1);
//        return;
//    }
//    RETURN_STRINGL(data, length, 0);
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, get, 0)
//    METHOD_ARG_LONG(bytes, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::getAll() : string */
//CLASS_METHOD(ION_Stream, getAll) {
//    ion_stream * stream = getThisInstance();
//    size_t length;
//    char * data = NULL;
//
//    CHECK_STREAM_BUFFER(stream);
//    length = (long)ion_stream_input_length(stream);
//    data = ion_stream_read(stream, &length);
//    if(data == NULL) {
//        ThrowRuntime("Stream is unreachable", -1);
//        return;
//    }
//    RETURN_STRINGL(data, length, 0);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, getAll);
//
///** public function ION\Stream::getLine(string $token, $mode = self::MODE_TRIM_TOKEN, $max_length = 0) : string|bool */
//CLASS_METHOD(ION_Stream, getLine) {
//    ion_stream * stream = getThisInstance();
//    ion_stream_token token = empty_stream_token;
//    char * data;
//    long size;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("s|ll", &token.token, &token.token_length, &token.flags, &token.length);
//    token.flags &= ION_STREAM_TOKEN_MODE_MASK;
//    if(token.token_length == 0) {
//        RETURN_FALSE;
//    }
//
//    if(ion_stream_search_token(bufferevent_get_input(stream->buffer), &token) == FAILURE) {
//        ThrowRuntime("Failed to get internal buffer pointer for token_length/offset", -1);
//        return;
//    }
//
//    if(token.position == -1) {
//        RETURN_FALSE;
//    } else {
//        size = ion_stream_read_token(stream, &data, &token);
//        if(size == -1) {
//            RETURN_FALSE;
//        } else if(size == 0) {
//            RETVAL_EMPTY_STRING();
//        } else {
//            RETURN_STRINGL(data, size, 0);
//        }
//    }
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, getLine, 1)
//    METHOD_ARG_STRING(token, 0)
//    METHOD_ARG_LONG(mode, 0)
//    METHOD_ARG_LONG(max_length, 0)
//METHOD_ARGS_END()
//
//void _deferred_stream_await_dtor(void *object, zval *zdeferred TSRMLS_DC) {
//    ion_stream * stream = (ion_stream *) object;
//    bufferevent_setwatermark(stream->buffer, EV_READ, 0, stream->input_size);
//    if(stream->read) {
//        if(stream->token) {
//            efree(stream->token->token);
//            efree(stream->token);
//            stream->token = NULL;
//        }
//        zval_ptr_dtor(&stream->read);
//        stream->read = NULL;
//    }
//}
//
///** public function ION\Stream::await(int $bytes) : ION\Deferred */
//CLASS_METHOD(ION_Stream, await) {
//    ion_stream * stream = getThisInstance();
//    size_t length = 0;
//    size_t current = 0;
//    char * data = NULL;
//    zval * zdeferred = NULL;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("l", &length);
//    if(stream->read) {
//        ThrowLogic("Stream already reading", -1);
//        return;
//    }
//
//    current = ion_stream_input_length(stream);
//    zdeferred = ion_deferred_new_ex(NULL);
//    if(current >= length) {
//        data = ion_stream_read(stream, &length);
//        if(data == NULL) {
//            ThrowRuntime("Stream is unreachable", -1);
//            return;
//        }
//        ion_deferred_done_stringl(zdeferred, data, length, 0);
//    } else {
//        ion_deferred_store(zdeferred, stream, _deferred_stream_await_dtor);
//        stream->read = zdeferred;
//        stream->length = length;
//        bufferevent_setwatermark(stream->buffer, EV_READ, length, (stream->input_size >= length) ? stream->input_size : length);
//    }
//    RETURN_ZVAL(zdeferred, 1, 0);
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, await, 1)
//    METHOD_ARG_LONG(bytes, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::awaitLine(string $token, $mode = self::MODE_TRIM_TOKEN, $max_length = 0) : ION\Deferred */
//CLASS_METHOD(ION_Stream, awaitLine) {
//    ion_stream * stream = getThisInstance();
//    ion_stream_token token = empty_stream_token;
//    zval * zdeferred = NULL;
//    char * data;
//    long size;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("s|ll", &token.token, &token.token_length, &token.flags, &token.length);
//    token.flags &= ION_STREAM_TOKEN_MODE_MASK;
//    if(token.token_length == 0) {
//        pion_throw(ion_get_class(ION_InvalidArgumentException), "empty token", -1);
//        return;
//    }
//
//    if(ion_stream_search_token(bufferevent_get_input(stream->buffer), &token) == FAILURE) {
//        ThrowRuntime("Failed to get internal buffer pointer for token_length/offset", -1);
//        return;
//    }
//
//    zdeferred = ion_deferred_new_ex(NULL);
//
//    if(token.position == -1) { // not found
//        if(token.flags & ION_STREAM_TOKEN_LIMIT) {
//            ion_deferred_done_false(zdeferred);
//        } else {
//            ion_deferred_store(zdeferred, stream, _deferred_stream_await_dtor);
//            stream->read = zdeferred;
//            stream->token = emalloc(sizeof(ion_stream_token));
//            memcpy(stream->token, &token, sizeof(ion_stream_token));
//            stream->token->token = estrndup(token.token, (unsigned)token.token_length);
//        }
//    } else { // found
//        size = ion_stream_read_token(stream, &data, &token);
//        if(size == -1) {
//            if(EG(exception)) {
//                ion_deferred_exception_eg(stream->read);
//            } else {
//                ion_deferred_exception(stream->read, ION_Stream_RuntimeException(), "Stream corrupted: failed to read token from buffer", -1);
//            }
//        } else if(size == 0) {
//            ion_deferred_done_empty_string(zdeferred);
//        } else {
//            ion_deferred_done_stringl(zdeferred, data, size, 0);
//        }
//    }
//    RETURN_ZVAL(zdeferred, 1, 0);
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, awaitLine, 1)
//    METHOD_ARG_STRING(token, 0)
//    METHOD_ARG_LONG(mode, 0)
//    METHOD_ARG_LONG(max_length, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::awaitAll() : ION\Deferred */
//CLASS_METHOD(ION_Stream, awaitAll) {
//    ion_stream * stream = getThisInstance();
//    zval * zdeferred = NULL;
//    CHECK_STREAM_BUFFER(stream);
//    if(stream->read) {
//        ThrowLogic("Stream already reading", -1);
//        return;
//    }
//    zdeferred = ion_deferred_new_ex(NULL);
//    if(stream->state & ION_STREAM_STATE_EOF) {
//        ion_deferred_done_empty_string(zdeferred);
//    } else {
//        ion_deferred_store(zdeferred, stream, _deferred_stream_await_dtor);
//        stream->read = zdeferred;
//        stream->length = 0;
//    }
//    RETURN_ZVAL(zdeferred, 1, 0);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, awaitAll)
//
///** public function ION\Stream::close(bool $force = false) : self */
//CLASS_METHOD(ION_Stream, close) {
//    ion_stream * stream = getThisInstance();
//    zend_bool force = 0;
//
//    CHECK_STREAM_BUFFER(stream);
//    PARSE_ARGS("|b", &force);
//    if(stream->state & ION_STREAM_STATE_CLOSED) {
//        RETURN_THIS();
//    }
//    if((stream->state & ION_STREAM_STATE_FLUSHED) || force) {
//        ion_stream_close_fd(stream);
//    } else {
//        bufferevent_disable(stream->buffer, EV_READ);
//        stream->state |= ION_STREAM_STATE_CLOSE_ON_FLUSH;
//        RETURN_THIS();
//    }
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, close, 0)
//    METHOD_ARG_BOOL(force, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::onData(callable $callback) : self */
//CLASS_METHOD(ION_Stream, onData) {
//
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, onData, 1)
//    METHOD_ARG_CALLBACK(callback, 0, 1)
//METHOD_ARGS_END()
//
//
//void _deferred_stream_shutdown_dtor(void * object, zval * zdeferred TSRMLS_DC) {
//    ion_stream * stream = (ion_stream *) object;
//    if(stream->closing) {
//        zval_ptr_dtor(&stream->closing);
//        stream->closing = NULL;
//    }
//}
//
///** public function ION\Stream::awaitClosing() : ION\Deferred */
//CLASS_METHOD(ION_Stream, awaitShutdown) {
//    ion_stream * stream = getThisInstance();
//    zval * zdeferred = NULL;
//    CHECK_STREAM_BUFFER(stream);
//    zdeferred = ion_deferred_new_ex(NULL);
//    if(stream->state & ION_STREAM_STATE_CLOSED) {
//        ion_deferred_done(zdeferred, getThis());
//    } else {
//        ion_deferred_store(zdeferred, stream, _deferred_stream_shutdown_dtor);
//        stream->closing = zdeferred;
//    }
//    RETURN_ZVAL(zdeferred, 1, 0);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, awaitShutdown)
//
///** public function ION\Stream::ensureSSL(ION\SSL $ssl) : self */
//CLASS_METHOD(ION_Stream, ensureSSL) {
//
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, ensureSSL, 1)
//    METHOD_ARG(ssl, 0)
//METHOD_ARGS_END()
//
///** public function ION\Stream::isClosed() : int */
//CLASS_METHOD(ION_Stream, isClosed) {
//    ion_stream * stream = getThisInstance();
//    RETURN_LONG(stream->state & ION_STREAM_STATE_CLOSED);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, isClosed)
//
///** public function ION\Stream::isEnabled() : bool */
//CLASS_METHOD(ION_Stream, isEnabled) {
//    ion_stream * stream = getThisInstance();
//    RETURN_LONG(stream->state & ION_STREAM_STATE_ENABLED);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, isEnabled)
//
///** public function ION\Stream::isConnected() : bool */
//CLASS_METHOD(ION_Stream, isConnected) {
//    ion_stream * stream = getThisInstance();
//    RETURN_LONG(stream->state & ION_STREAM_STATE_CONNECTED);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, isConnected)
//
///** public function ION\Stream::getState() : int */
//CLASS_METHOD(ION_Stream, getState) {
//    ion_stream * stream = getThisInstance();
//    RETURN_LONG(stream->state);
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, getState)
//
//
///** public function ION\Stream::__debugInfo() : void */
//CLASS_METHOD(ION_Stream, __debugInfo) {
//    ion_stream * stream = getThisInstance();
//    zval * read         = NULL;
//    char * address      = NULL;
//
//    array_init(return_value);
//    add_assoc_long(return_value, "fd", bufferevent_getfd(stream->buffer));
//    if(stream->state & ION_STREAM_STATE_PAIR) {
//        add_assoc_string(return_value, "type", "pair-socket", 1);
//    } else if(stream->state & ION_STREAM_STATE_SOCKET) {
//        add_assoc_string(return_value, "type", "socket", 1);
//    } else {
//        add_assoc_string(return_value, "type", "pipe", 1);
//    }
//    add_assoc_bool(return_value, "connected",    stream->state & ION_STREAM_STATE_CONNECTED);
//    add_assoc_bool(return_value, "ssl",          0);
//    add_assoc_long(return_value, "input_bytes",  ion_stream_input_length(stream));
//    add_assoc_long(return_value, "output_bytes", ion_stream_output_length(stream));
//
//    address = ion_stream_get_name_self(stream);
//    if(address) {
//        add_assoc_string(return_value, "local_name", address, 0);
//    } else {
//        add_assoc_bool(return_value,   "local_name",          0);
//    }
//
//    address = ion_stream_get_name_remote(stream);
//    if(address) {
//        add_assoc_string(return_value, "remote_peer", address, 0);
//    } else {
//        add_assoc_bool(return_value,   "remote_peer",          0);
//    }
//
//    if(stream->state & ION_STREAM_STATE_CLOSED) {
//        if(stream->state & ION_STREAM_STATE_EOF) {
//            add_assoc_string(return_value, "closed", "eof", 1);
//        } else if (stream->state & ION_STREAM_STATE_ERROR) {
//            add_assoc_string(return_value, "closed", "error", 1);
//        } else {
//            add_assoc_string(return_value, "closed", "shutdown", 1);
//        }
//
//    } else {
//        add_assoc_bool(return_value, "closed", 0);
//    }
//    if(stream->read) {
//        ALLOC_INIT_ZVAL(read);
//        array_init(read);
//        if(stream->token) {
//            add_assoc_stringl(read, "token", stream->token->token, (uint)stream->token->token_length, 1);
//            add_assoc_long(read, "max_bytes", stream->token->length);
//            add_assoc_long(read, "scanned_bytes", stream->token->offset);
//            if(stream->token->flags & ION_STREAM_MODE_TRIM_TOKEN) {
//                add_assoc_string(read, "mode", "trim_token", 1);
//            } else if(stream->token->flags & ION_STREAM_MODE_WITH_TOKEN) {
//                add_assoc_string(read, "mode", "with_token", 1);
//            } else {
//                add_assoc_string(read, "mode", "without_token", 1);
//            }
//        } else if(stream->length) {
//            add_assoc_long(read, "bytes", stream->length);
//        } else {
//            add_assoc_bool(read, "all",   1);
//        }
//        add_assoc_zval(return_value, "read", read);
//    } else {
//        add_assoc_bool(return_value, "read", 0);
//    }
//
//    add_assoc_bool(return_value, "await_flush",    stream->flush ? 1 : 0);
//    add_assoc_bool(return_value, "await_connect",  stream->connect ? 1 : 0);
//    add_assoc_bool(return_value, "await_shutdown", stream->closing ? 1 : 0);
//
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, __debugInfo)
//
///** public function ION\Stream::__destruct() : void */
//CLASS_METHOD(ION_Stream, __destruct) {
////    PHPDBG("stream __destruct start")
//
////    zend_error(E_NOTICE, "Stream destruct");
//    ion_stream * stream = getThisInstance();
//    if(stream->flush) {
//        ion_deferred_cancel(stream->flush, "The stream shutdown by the destructor");
//    }
//    if(stream->read) {
//        ion_deferred_cancel(stream->read, "The stream shutdown by the destructor");
//    }
//    if(stream->connect) {
//        ion_deferred_cancel(stream->connect, "The stream shutdown by the destructor");
//    }
//    if(stream->state & ION_STREAM_STATE_ENABLED) {
//        bufferevent_disable(stream->buffer, EV_READ | EV_WRITE);
//        stream->state &= ~ION_STREAM_STATE_ENABLED;
//    }
////    PHPDBG("stream __destruct done")
//
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, __destruct)
//
//
/** public function ION\Stream::getRemotePeer() : string */
CLASS_METHOD(ION_Stream, getRemotePeer) {
    ion_stream  * stream = get_this_instance(ion_stream);
    zend_string * remote_name;
    if(stream->buffer == NULL) {
        RETURN_FALSE;
    }
    if(stream->state & ION_STREAM_STATE_SOCKET) {
        if(ion_stream_is_valid_fd(stream)) {
            RETURN_FALSE;
        }
        if(stream->state & ION_STREAM_STATE_PAIR) {
            RETURN_STRING("twin");
        }
        remote_name = ion_stream_get_name_remote(Z_OBJ_P(getThis()));
        if(remote_name) {
            RETURN_STR(remote_name);
        } else {
            RETURN_FALSE;
        }
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, getRemotePeer)

/** public function ION\Stream::getLocalName() : string */
CLASS_METHOD(ION_Stream, getLocalName) {
    ion_stream * stream = get_this_instance(ion_stream);
    zend_string * local_name;
    if(stream->buffer == NULL) {
        RETURN_FALSE;
    }
    if(stream->state & ION_STREAM_STATE_SOCKET) {
        if(ion_stream_is_valid_fd(stream)) {
            RETURN_FALSE;
        }
        if(stream->state & ION_STREAM_STATE_PAIR) {
            RETURN_STRING("twin")
        }
        local_name = ion_stream_get_name_self(Z_OBJ_P(getThis()));
        if(local_name) {
            RETURN_STR(local_name);
        } else {
            RETURN_FALSE;
        }
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Stream, getLocalName)

///** public function ION\Stream::__toString() : string */
//CLASS_METHOD(ION_Stream, __toString) {
//    ion_stream * stream = getThisInstance();
//    char       * address_combined;
//    char       * address_remote;
//    char       * address_local;
//    if(stream->buffer == NULL) {
//        RETURN_STRING("stream:empty", 1);
//    }
//    if(stream->state & ION_STREAM_STATE_SOCKET) {
//        if(ion_stream_is_valid_fd(stream)) {
//            RETURN_STRING("stream:invalid", 1);
//        }
//        address_local = ion_stream_get_name_self(stream);
//        address_remote  = ion_stream_get_name_remote(stream);
//        if(address_remote == NULL) {
//            address_remote = estrdup("undefined");
//        }
//        if(address_local == NULL) {
//            address_local = estrdup("undefined");
//        }
//
//        spprintf(&address_combined, 0, "stream:socket(%s->%s)", address_local, address_remote);
//        RETVAL_STRING(address_combined, 1);
//        efree(address_combined);
//        if(address_local) {
//            efree(address_local);
//        }
//        if(address_remote) {
//            efree(address_remote);
//        }
//    } else if(stream->state & ION_STREAM_STATE_PAIR) {
//        RETURN_STRING("stream:twin", 1);
//    } else {
//        RETURN_STRING("stream:pipe", 1);
//    }
//}
//
//METHOD_WITHOUT_ARGS(ION_Stream, __toString)
//
//#ifdef ION_DEBUG
///** public function ION\Stream::appendToInput(string $data) : self */
//CLASS_METHOD(ION_Stream, appendToInput) {
//    char *data;
//    int data_len = 0;
//    ion_stream * stream = getThisInstance();
//    struct evbuffer * input;
//
//    CHECK_STREAM(stream);
//    PARSE_ARGS("s", &data, &data_len);
//
//    if(!data_len) {
//        RETURN_THIS();
//    }
//
//    input = bufferevent_get_input(stream->buffer);
//
//    evbuffer_unfreeze(input, 0);
//    if(evbuffer_add(input, (const void *)data, (size_t)data_len)) {
//        ThrowRuntime("Failed to append data to input", 1);
//        return;
//    }
//    stream->state &= ~ION_STREAM_STATE_FLUSHED;
//    RETURN_THIS();
//}
//
//METHOD_ARGS_BEGIN(ION_Stream, appendToInput, 1)
//    METHOD_ARG(data, 0)
//METHOD_ARGS_END()
//#endif

CLASS_METHODS_START(ION_Stream)
    METHOD(ION_Stream, resource,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, pair,            ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Stream, socket,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#ifdef ION_DEBUG
//    METHOD(ION_Stream, _input,          ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, _output,         ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, _notify,         ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, appendToInput,   ZEND_ACC_PUBLIC)
#else
    METHOD(ION_Stream, _incoming,       ZEND_ACC_PRIVATE)
    METHOD(ION_Stream, _empty,          ZEND_ACC_PRIVATE)
    METHOD(ION_Stream, _notify,         ZEND_ACC_PRIVATE)
#endif
//    METHOD(ION_Stream, enable,          ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, disable,         ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, awaitConnection, ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, setTimeouts,     ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, setPriority,     ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, setInputSize,    ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, write,           ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, sendFile,        ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, flush,           ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, search,          ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, getSize,         ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, get,             ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, getAll,          ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, getLine,         ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, await,           ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, awaitAll,        ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, awaitLine,       ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, close,           ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, onData,          ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, awaitShutdown,   ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, ensureSSL,       ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getRemotePeer,   ZEND_ACC_PUBLIC)
    METHOD(ION_Stream, getLocalName,    ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, isClosed,        ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, isEnabled,       ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, isConnected,     ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, getState,        ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, __destruct,      ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, __toString,      ZEND_ACC_PUBLIC)
//    METHOD(ION_Stream, __debugInfo,     ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Stream) {
    pion_register_class(ION_Stream, "ION\\Stream", ion_stream_init, CLASS_METHODS(ION_Stream));
    pion_init_std_object_handlers(ION_Stream);
    pion_set_object_handler(ION_Stream, free_obj, ion_stream_free);
    pion_set_object_handler(ION_Stream, clone_obj, NULL);

    PION_CLASS_CONST_LONG(ION_Stream, "MODE_TRIM_TOKEN",    ION_STREAM_MODE_TRIM_TOKEN);
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_WITH_TOKEN",    ION_STREAM_MODE_WITH_TOKEN);
    PION_CLASS_CONST_LONG(ION_Stream, "MODE_WITHOUT_TOKEN", ION_STREAM_MODE_WITHOUT_TOKEN);

    PION_CLASS_CONST_LONG(ION_Stream, "STATE_SOCKET", ION_STREAM_STATE_SOCKET);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_PAIR",   ION_STREAM_STATE_PAIR);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_PIPE",   ION_STREAM_STATE_PIPE);

//    PION_CLASS_CONST_LONG(ION_Stream, "STATE_READING", ION_STREAM_STATE_READING);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_FLUSHED",   ION_STREAM_STATE_FLUSHED);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_HAS_DATA",  ION_STREAM_STATE_HAS_DATA);

    PION_CLASS_CONST_LONG(ION_Stream, "STATE_ENABLED",   ION_STREAM_STATE_ENABLED);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_CONNECTED", ION_STREAM_STATE_CONNECTED);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_EOF",       ION_STREAM_STATE_EOF);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_ERROR",     ION_STREAM_STATE_ERROR);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_SHUTDOWN",  ION_STREAM_STATE_SHUTDOWN);
    PION_CLASS_CONST_LONG(ION_Stream, "STATE_CLOSED",    ION_STREAM_STATE_CLOSED);

//    PION_CLASS_CONST_LONG(ION_Stream, "NAME_HOST",     ION_STREAM_NAME_HOST);
//    PION_CLASS_CONST_LONG(ION_Stream, "NAME_ADDRESS",  ION_STREAM_NAME_ADDRESS);
//    PION_CLASS_CONST_LONG(ION_Stream, "NAME_PORT",     ION_STREAM_NAME_PORT);

    PION_CLASS_CONST_LONG(ION_Stream, "INPUT",  EV_READ);
    PION_CLASS_CONST_LONG(ION_Stream, "OUTPUT", EV_WRITE);
    PION_CLASS_CONST_LONG(ION_Stream, "BOTH",   EV_WRITE | EV_READ);
    return SUCCESS;
}
