#include "ion.h"
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>


zend_string * ion_stream_get_name_self(ion_stream * stream) {
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

zend_string * ion_stream_get_name_remote(ion_stream * stream) {
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

zend_string * ion_stream_describe(ion_stream * stream) {
    zend_string * describe = NULL;
    zend_string * address_remote = NULL;
    zend_string * address_local = NULL;

    if(stream->buffer == NULL) {
        return zend_string_init(STRARGS("Stream"), 0);
    }
    if (stream->state & ION_STREAM_RESERVED) {
        switch(bufferevent_getfd(stream->buffer)) {
            case STDIN_FILENO:
                return zend_string_init(STRARGS("Stream(stdin)"), 0);
            case STDOUT_FILENO:
                return zend_string_init(STRARGS("Stream(stdout)"), 0);
            case STDERR_FILENO:
                return zend_string_init(STRARGS("Stream(stderr)"), 0);
            default:
                ZEND_ASSERT(0);
        }
    }
    if(stream->state & ION_STREAM_STATE_SOCKET) {
        if(ion_stream_is_valid_fd(stream)) {
            return zend_string_init(STRARGS("Stream"), 0);
        }
        address_local   = ion_stream_get_name_self(stream);
        address_remote  = ion_stream_get_name_remote(stream);
        if(address_remote == NULL) {
            address_remote = zend_string_init(STRARGS("undefined"), 0);
        }
        if(address_local == NULL) {
            address_remote = zend_string_init(STRARGS("undefined"), 0);
        }

        if(stream->state & ION_STREAM_FROM_PEER) {
            describe = strpprintf(MAX_DOMAIN_LENGTH, "Stream(%s<-%s)", address_local->val, address_remote->val);
        } else if(stream->state & ION_STREAM_FROM_ME) {
            describe = strpprintf(MAX_DOMAIN_LENGTH, "Stream(%s->%s)", address_local->val, address_remote->val);
        } else {
            describe = strpprintf(MAX_DOMAIN_LENGTH, "Stream(%s<->%s)", address_local->val, address_remote->val);
        }
        zend_string_release(address_local);
        zend_string_release(address_remote);
        return describe;
    } else if(stream->state & ION_STREAM_STATE_PAIR) {
        return zend_string_init(STRARGS("Stream(twin)"), 0);
    } else {
        return zend_string_init(STRARGS("Stream(pipe)"), 0);
    }
}

long ion_stream_search_token(struct evbuffer * buffer, ion_stream_token * token) {
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
    if(token->offset >= current_size || ZSTR_LEN(token->token) > current_size) {
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
        ptr_result = evbuffer_search_range(buffer, ZSTR_VAL(token->token), ZSTR_LEN(token->token), &ptr_start, &ptr_end);
    } else {
        ptr_result = evbuffer_search(buffer, ZSTR_VAL(token->token), ZSTR_LEN(token->token), &ptr_start);
    }
    if(token->length > 0 && current_size >= token->length) {
        token->flags |= ION_STREAM_TOKEN_LIMIT;
    }
    token->offset = (zend_long)(current_size - ZSTR_LEN(token->token) + 1);
    token->position = (long)ptr_result.pos;
    return SUCCESS;
}

zend_bool ion_buffer_pair(ion_buffer ** one, ion_buffer ** two) {
    evutil_socket_t  socks[2] = {-1, -1};
    ion_buffer     * pair[2];

    if(evutil_socketpair(LOCAL_SOCKETPAIR_AF, SOCK_STREAM, 0, socks) == FAILURE) {
        return false;
    }

    pair[0] = bufferevent_socket_new(GION(base), socks[0], BEV_OPT_CLOSE_ON_FREE);
    pair[1] = bufferevent_socket_new(GION(base), socks[1], BEV_OPT_CLOSE_ON_FREE);

    if(bufferevent_enable(pair[0], EV_READ | EV_WRITE) == FAILURE ||
       bufferevent_enable(pair[1], EV_READ | EV_WRITE) == FAILURE) {
        bufferevent_free(pair[0]);
        bufferevent_free(pair[1]);
        zend_throw_exception(ion_ce_ION_StreamException, "Failed to enable stream", 0);
        return false;
    }

    *one = pair[0];
    *two = pair[1];

    return true;
}

zend_string * ion_buffer_read_all(ion_buffer * buffer) {
    size_t incoming_length = evbuffer_get_length(bufferevent_get_input(buffer));
    zend_string * data;

    if(!incoming_length) {
        return ZSTR_EMPTY_ALLOC();
    }

    data = zend_string_alloc(incoming_length, 0);
    ZSTR_LEN(data) = bufferevent_read(buffer, ZSTR_VAL(data), incoming_length);
    if (ZSTR_LEN(data) > 0) {
        ZSTR_VAL(data)[ZSTR_LEN(data)] = '\0';
        return data;
    } else {
        zend_string_free(data);
        return NULL;
    }
}

zend_bool ion_stream_write(ion_stream * stream, zend_string * data) {
    if(bufferevent_write(stream->buffer, ZSTR_VAL(data), ZSTR_LEN(data)) == FAILURE) {
        return false;
    }

    if(ion_stream_output_length(stream) && (stream->state & ION_STREAM_STATE_FLUSHED)) {
        stream->state &= ~ION_STREAM_STATE_FLUSHED;
    }

    return true;
}

zend_object * ion_stream_get_exception(ion_stream * stream, ion_buffer * bev) {
    zend_ulong         error_ulong = 0;
    int                error_int = 0;
    const char       * error_message;
    zend_object      * exception;
    void             * exception_ce;
    zend_string      * desc = ion_stream_describe(stream);

    if((error_ulong =  bufferevent_get_openssl_error(bev))) { // problem with openssl connection
        error_message = ERR_error_string(error_ulong, NULL);
        exception_ce = ion_ce_ION_CryptoException;
    } else if((error_int =  bufferevent_socket_get_dns_error(bev))) { // DNS problem
        error_message = evutil_gai_strerror(error_int);
        exception_ce = ion_ce_ION_DNSException;
    } else if((error_int = EVUTIL_SOCKET_ERROR())) { // socket problem
        error_message = evutil_socket_error_to_string(error_int);
        exception_ce = ion_ce_ION_StreamException;
    } else { // magic problem
        error_message = "stream corrupted";
        exception_ce = ion_ce_ION_StreamException;
    }

    exception = pion_exception_new_ex(
            exception_ce, 0,
            "%s error: %s", desc->val, error_message
    );
    zend_string_release(desc);
    return exception;
}

