#include "stream.h"
#include "ion.h"

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

//zval ion_stream_read_token(zval * result, ion_stream_token * token) {
//
//    token->flags &= ION_STREAM_TOKEN_MODE_MASK;
//    if(ZSTR_LEN(token->token) == 0) {
//        zend_throw_exception(ion_ce_InvalidArgumentException, "Token can't be empty", 0);
//        return;
//    }
//
//    if(stream->read) {
//        if(stream->token
//           && zend_string_equals(stream->token->token, token.token)
//           && (stream->token->flags & ION_STREAM_TOKEN_MODE_MASK) == token.flags
//           && stream->token->length == token.length) {
//            obj_add_ref(stream->read);
//            RETURN_OBJ(stream->read);
//        } else {
//            zend_throw_exception(ion_ce_ION_InvalidUsageException, "Stream locked for reading: already in the process of reading", 0);
//            return;
//        }
//    }
//
//    if(ion_stream_search_token(bufferevent_get_input(stream->buffer), &token) == FAILURE) {
//        zend_throw_exception(ion_ce_ION_StreamException, "Failed to get internal buffer pointer for token_length/offset", 0);
//        return;
//    }
//
//    if(token.position == -1) {
//        if(token.flags & ION_STREAM_TOKEN_LIMIT) {
//            RETURN_FALSE;
//        } else {
//            zend_object * deferred = ion_promisor_deferred_new_ex(NULL);
//            ion_promisor_store(deferred, stream);
//            ion_promisor_dtor(deferred, _ion_stream_read_dtor);
//            stream->read = deferred;
//            stream->token = emalloc(sizeof(ion_stream_token));
//            memcpy(stream->token, &token, sizeof(ion_stream_token));
//            stream->token->token = zend_string_copy(token.token);
//            obj_add_ref(deferred);
//            RETURN_OBJ(deferred);
//        }
//    } else {
//        data = ion_stream_read_token(stream, &token);
//        if(data == NULL) {
//            RETURN_FALSE;
//        } else {
//            RETURN_STR(data);
//        }
//    }
//}