#include "ion.h"
#include <event2/listener.h>
#include <sys/un.h>
#include <event2/bufferevent_ssl.h>


zend_object_handlers ion_oh_ION_Listener;
zend_class_entry * ion_ce_ION_Listener;
zend_object_handlers ion_oh_ION_ListenerException;
zend_class_entry * ion_ce_ION_ListenerException;

void ion_listener_release(ion_listener * listener) {
    if(listener->listener) {
        evconnlistener_disable(listener->listener);
        evconnlistener_free(listener->listener);
        listener->listener = NULL;
    }
    if(listener->accept) {
        ion_object_release(listener->accept);
        listener->accept = NULL;
    }
    listener->flags &= ~ION_STREAM_STATE_ENABLED;
}

zend_object * ion_listener_init(zend_class_entry * ce) {
    ion_listener * listener = ion_alloc_object(ce, ion_listener);
    return ion_init_object(ION_OBJECT_ZOBJ(listener), ce, &ion_oh_ION_Listener);
}

void ion_listener_free(zend_object * object) {
    ion_listener * listener = ION_ZOBJ_OBJECT(object, ion_listener);
    zend_object_std_dtor(object);
    ion_listener_release(listener);
    if(listener->name) {
        zend_string_release(listener->name);
    }
    if(listener->accept) {
        ion_object_release(listener->accept);
    }
    if(listener->encrypt) {
        ion_object_release(listener->encrypt);
    }
}

//void ion_listener_enable(zend_object * listener_obj, zend_bool state) {
//    ion_listener * listener = get_object_instance(listener_obj, ion_listener);
//    if(listener->listener) {
//        if(state) {
//            if(!(listener->flags & ION_STREAM_STATE_ENABLED)) {
//                evconnlistener_enable(listener->listener);
//                listener->flags |= ION_STREAM_STATE_ENABLED;
//            }
//        } else {
//            if((listener->flags & ION_STREAM_STATE_ENABLED)) {
//                evconnlistener_disable(listener->listener);
//                listener->flags &= ~ION_STREAM_STATE_ENABLED;
//            }
//        }
//    }
//}

static void _ion_listener_accept(ion_evlistener * l, evutil_socket_t fd, struct sockaddr * addr, int addr_len, void * ctx) {
    ION_CB_BEGIN();
    ion_listener * listener = ctx;
    zend_object  * stream = NULL;
    ion_stream   * istream = NULL;
    ion_buffer   * buffer = NULL;
    int            state = ION_STREAM_STATE_ENABLED | ION_STREAM_STATE_SOCKET | ION_STREAM_STATE_CONNECTED | ION_STREAM_FROM_PEER;

    if(listener->accept) {
        if(listener->encrypt) {
            SSL * ssl_handler = ion_crypto_server_stream_handler(listener->encrypt);
            if(!ssl_handler) {
                zend_error(E_WARNING, ERR_ION_LISTENER_SSL_ERROR, listener->name->val);
                evutil_closesocket(fd);

            } else {
                buffer = bufferevent_openssl_socket_new(GION(base), fd, ssl_handler,
                                                        BUFFEREVENT_SSL_ACCEPTING,
                                                        BEV_OPT_CLOSE_ON_FREE);
                state |= ION_STREAM_ENCRYPTED;
                SSL_set_ex_data(ssl_handler, GION(ssl_index), listener->encrypt);
            }
        } else {
            buffer = bufferevent_socket_new(GION(base), fd, BEV_OPT_CLOSE_ON_FREE);
        }
        if(buffer) {
            if(listener->stream_class) {
                stream = ion_stream_new_ex(buffer, state, listener->stream_class);
            } else {
                stream = ion_stream_new(buffer, state);
            }
            istream = ION_ZOBJ_OBJECT(stream, ion_stream);
            if(listener->encrypt) {
                ion_object_addref(listener->encrypt);
                istream->encrypt = listener->encrypt;
            }

            if(listener->name) {
                istream->name_self = zend_string_copy(listener->name);
            }
            if(listener->flags & ION_STREAM_NAME_UNIX) {
                istream->name_remote = zend_string_init("pipe", sizeof("pipe"), 0);
            } else {
                pion_net_addr_to_name(addr, (socklen_t)addr_len, &istream->name_remote);
            }
            istream->state |= (listener->flags & ION_STREAM_NAME_MASK);
            if(listener->accept) {
                zval zstream;
                ZVAL_OBJ(&zstream, stream);
                ion_promisor_done_object(listener->accept, stream);
            }
            zend_object_release(stream);
        }
    } else {
        evutil_closesocket(fd);
    }
    ION_CB_END();
}

static void _ion_listener_error(ion_evlistener * l, void * ctx) {
    ION_CB_BEGIN();
    ion_listener * listener = ctx;
    int err = EVUTIL_SOCKET_ERROR();
    zend_error(E_WARNING, ERR_ION_LISTENER_GOT_AN_ERROR,
               err, evutil_socket_error_to_string(err), listener->name);
    evconnlistener_disable(listener->listener);
    evconnlistener_free(listener->listener);
    listener->listener = NULL;
    listener->flags &= ~ION_STREAM_STATE_ENABLED;
    ION_CB_END();
}

/** public function ION\Listener::__construct(string $listen, int $back_log = -1) : self */
CLASS_METHOD(ION_Listener, __construct) {
    zend_string * listen_address = NULL;
    zend_long back_log = -1;
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(listen_address)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(back_log)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(listen_address->val[0] != '/') { // ipv4, ipv6, hostname
        struct sockaddr_storage sock;
        socklen_t sock_len = sizeof(struct sockaddr_storage);
        int result = evutil_parse_sockaddr_port(listen_address->val, (struct sockaddr *)&sock, (int *)&sock_len);
        if(result == SUCCESS) {
            if(sock.ss_family == AF_INET) {
                listener->flags |= ION_STREAM_NAME_IPV4;
            } else if(sock.ss_family == AF_INET6) {
                listener->flags |= ION_STREAM_NAME_IPV6;
            } else {
                zend_throw_exception_ex(ion_ce_ION_InvalidUsageException, 0, ERR_ION_LISTENER_UNSUPPORTED_ADDRESS, sock.ss_family);
                return;
            }
        } else {
            zend_throw_exception_ex(ion_ce_InvalidArgumentException, 0, ERR_ION_LISTENER_INVALID_FORMAT, listen_address->val);
            return;
        }
        listener->listener = evconnlistener_new_bind(
            GION(base),
            _ion_listener_accept,
            listener,
            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_REUSEABLE_PORT | LEV_OPT_CLOSE_ON_EXEC,
            (int) back_log,
            (struct sockaddr *) &sock,
            sock_len
        );
        if(listener->listener) {
            pion_net_sock_name(evconnlistener_get_fd(listener->listener), PION_NET_NAME_LOCAL, &listener->name);
        }
    } else { // unix socket
        evutil_socket_t fd;
        struct sockaddr_un local;
        socklen_t sock_len = sizeof(struct sockaddr_un);

        listener->flags |= ION_STREAM_NAME_UNIX;
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(fd < 0) {
            zend_throw_exception_ex(ion_ce_ION_RuntimeException, errno, ERR_ION_LISTENER_FAILED_OPEN_SOCKET, listen_address->val, evutil_socket_error_to_string(errno));
            return;
        }
        memset(&local, 0, sizeof(local));
//        local.sun_family = AF_UNIX;
        local.sun_family = AF_LOCAL;
        strncpy(local.sun_path, listen_address->val, 100);
        unlink(local.sun_path);
        if(bind(fd, (struct sockaddr *)&local, sock_len) == FAILURE) {
            zend_throw_exception_ex(ion_ce_ION_RuntimeException, errno, ERR_ION_LISTENER_FAILED_LISTEN_SOCKET, listen_address->val, evutil_socket_error_to_string(errno));
            return;
        }
        listener->listener = evconnlistener_new(GION(base), _ion_listener_accept, listener,
             LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC, (int)back_log, fd);
        listener->name = zend_string_copy(listen_address);
    }

    if(listener->listener) {
        evconnlistener_set_error_cb(listener->listener, _ion_listener_error);
        evconnlistener_enable(listener->listener);
        listener->flags |= ION_STREAM_STATE_ENABLED;
    } else {
        zend_throw_exception_ex(ion_ce_ION_RuntimeException, errno, ERR_ION_LISTENER_FAILED_LISTEN_SOCKET, listen_address->val, evutil_socket_error_to_string(errno));
        return;
    }
}

METHOD_ARGS_BEGIN(ION_Listener, __construct, 1)
    ARGUMENT(listen, IS_STRING)
    ARGUMENT(back_log, IS_LONG)
METHOD_ARGS_END();

/** public function ION\Listener::setStreamClass(string $classname) : self */
CLASS_METHOD(ION_Listener, setStreamClass) {
    zend_class_entry * ce = ion_ce_ION_Stream;
    ion_listener     * listener = ION_THIS_OBJECT(ion_listener);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_CLASS(ce)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    listener->stream_class = ce;
}

METHOD_ARGS_BEGIN(ION_Listener, setStreamClass, 1)
    ARGUMENT(classname, IS_STRING)
METHOD_ARGS_END();


/** public function ION\Listener::whenAccepted() : ION\Sequence */
CLASS_METHOD(ION_Listener, whenAccepted) {
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);

    if(!listener->accept) {
        listener->accept = ion_promisor_sequence_new(NULL);
    }

    ion_object_addref(listener->accept);
    RETURN_ION_OBJ(listener->accept);
}

METHOD_WITHOUT_ARGS(ION_Listener, whenAccepted);


/** public function ION\Listener::encrypt(ION\Crypto $ssl) : self */
CLASS_METHOD(ION_Listener, encrypt) {
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);
    zval         * zssl = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT(zssl)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    zval_add_ref(zssl);
    listener->encrypt = ION_ZVAL_OBJECT_P(zssl, ion_crypto);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Listener, encrypt, 1)
    ARGUMENT_OBJECT(ssl, ION\\Crypto, 0)
METHOD_ARGS_END()


/** public function ION\Listener::enable() : self */
CLASS_METHOD(ION_Listener, enable) {
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);
    if(listener->listener && !(listener->flags & ION_STREAM_STATE_ENABLED)) {
        evconnlistener_enable(listener->listener);
        listener->flags |= ION_STREAM_STATE_ENABLED;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Listener, enable);

/** public function ION\Listener::disable() : self */
CLASS_METHOD(ION_Listener, disable) {
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);
    if(listener->listener && (listener->flags & ION_STREAM_STATE_ENABLED)) {
        evconnlistener_disable(listener->listener);
        listener->flags &= ~ION_STREAM_STATE_ENABLED;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Listener, disable);

/** public function ION\Listener::shutdown() : self */
CLASS_METHOD(ION_Listener, shutdown) {
    ion_listener_release(ION_THIS_OBJECT(ion_listener));
}

METHOD_WITHOUT_ARGS(ION_Listener, shutdown);

/** public function ION\Listener::__toString() : string */
CLASS_METHOD(ION_Listener, __toString) {
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);
    RETURN_STR(zend_string_copy(listener->name));
}

METHOD_WITHOUT_ARGS(ION_Listener, __toString);

/** public function ION\Listener::getName() : string */
CLASS_METHOD(ION_Listener, getName) {
    ion_listener * listener = ION_THIS_OBJECT(ion_listener);
    RETURN_STR(zend_string_copy(listener->name));
}

METHOD_WITHOUT_ARGS(ION_Listener, getName);


METHODS_START(methods_ION_Listener)
    METHOD(ION_Listener, __construct,    ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, setStreamClass, ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, encrypt,        ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, whenAccepted,   ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, enable,         ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, disable,        ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, shutdown,       ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, getName,        ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, __toString,     ZEND_ACC_PUBLIC)
METHODS_END;


PHP_MINIT_FUNCTION(ION_Listener) {
    ion_register_class(ion_ce_ION_Listener, "ION\\Listener", ion_listener_init, methods_ION_Listener);
    ion_init_object_handlers(ion_oh_ION_Listener);
    ion_oh_ION_Listener.free_obj  = ion_listener_free;
    ion_oh_ION_Listener.clone_obj = NULL;
    ion_oh_ION_Listener.offset    = ion_offset(ion_listener);

    ion_register_exception(ion_ce_ION_ListenerException, ion_ce_ION_RuntimeException, "ION\\ListenerException");

    return SUCCESS;
}
