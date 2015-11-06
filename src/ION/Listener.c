#include "../pion.h"
#include <ext/standard/url.h>
#include <event2/listener.h>


zend_object_handlers ion_oh_ION_Listener;
zend_class_entry * ion_ce_ION_Listener;

void ion_listener_release(zend_object * object) {
    ion_listener * listener = get_object_instance(object, ion_listener);
    if(listener->listener) {
        evconnlistener_disable(listener->listener);
        evconnlistener_free(listener->listener);
        listener->listener = NULL;
    }
    listener->flags &= ~ION_STREAM_STATE_ENABLED;
}


zend_object * ion_listener_init(zend_class_entry * ce) {
    ion_promisor * listener = ecalloc(1, sizeof(ion_listener));
    RETURN_INSTANCE(ION_Listener, listener);
}

void ion_listener_free(zend_object * object) {
    ion_listener * listener = get_object_instance(object, ion_listener);
    zend_object_std_dtor(object);
    ion_listener_release(object);
    if(listener->name) {
        zend_string_release(listener->name);
    }
    if(listener->on_connect) {
        zend_object_release(listener->on_connect);
    }
    if(listener->ssl) {
        zend_object_release(listener->ssl);
    }
}

static void _ion_listener_accept(ion_evlistener * l, evutil_socket_t fd, struct sockaddr * addr, int addr_len, void * ctx) {
    ion_listener * listener = get_object_instance(ctx, ion_listener);
    zend_object  * stream = NULL;
    ion_stream   * istream = NULL;
    zval           zstream;

    if(listener->on_connect) {
        bevent * buffer = bufferevent_socket_new(ION(base), fd, BEV_OPT_CLOSE_ON_FREE);
        stream = ion_stream_new(buffer, ION_STREAM_STATE_ENABLED
                                        | ION_STREAM_STATE_SOCKET
                                        | ION_STREAM_STATE_CONNECTED
                                        | ION_STREAM_DIRECT_INCOMING);
        istream = get_object_instance(stream, ion_stream);
        if(listener->name) {
            istream->name_self = zend_string_copy(listener->name);
        }
        int type  = pion_net_addr_to_name(addr, (socklen_t)addr_len, &istream->name_remote);
        istream->state |= (type & ION_STREAM_NAME_MASK);
        ZVAL_OBJ(&zstream, stream);
        ion_promisor_sequence_invoke(listener->on_connect, &zstream);
        zend_object_release(stream);
    } else {
        evutil_closesocket(fd);
    }
    ION_CHECK_LOOP();
}

static void _ion_listener_error(ion_evlistener * l, void * ctx) {
    ion_listener * listener = get_object_instance(ctx, ion_listener);
    int err = EVUTIL_SOCKET_ERROR();
    zend_error(E_WARNING, "Got an error %d (%s) on the listener %s. Shutting down listener.",
               err, evutil_socket_error_to_string(err), listener->name);
    evconnlistener_disable(listener->listener);
    evconnlistener_free(listener->listener);
    listener->listener = NULL;
    listener->flags &= ~ION_STREAM_STATE_ENABLED;
    ION_CHECK_LOOP();
}

/** public function ION\Listener::__construct(string $listen, int $back_log = -1) : self */
CLASS_METHOD(ION_Listener, __construct) {
    zend_string * listen = NULL;
    zend_long back_log = -1;
    ion_listener * listener = get_this_instance(ion_listener);
    php_url * resource;
    struct sockaddr sock;
    int sock_len = sizeof(struct sockaddr);

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(listen)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(back_log)
    ZEND_PARSE_PARAMETERS_END();

    resource = php_url_parse_ex(listen->val, listen->len);
    if(resource->host) { // ipv4, ipv6, hostname
        int result = evutil_parse_sockaddr_port(resource->host, &sock, &sock_len);
        if(result == SUCCESS) {
            if(sock.sa_family == AF_INET) {
                struct sockaddr_in * sin = (struct sockaddr_in *) &sock;
                sin->sin_port = ntohs(resource->port);
            } else if(sock.sa_family == AF_INET6) {
                struct sockaddr_in6 * sin6 = (struct sockaddr_in6 *) &sock;
                sin6->sin6_port = ntohs(resource->port);
            } else {
                zend_throw_exception_ex(ion_class_entry(ION_InvalidUsageException), 0, "Address family %d not supported by protocol family", sock.sa_family);
            }
        } else {
            // TODO: try to resolve domain
        }
    } else if(resource->path) { // unix socket
//        struct sockaddr_un * unix = (struct sockaddr_un *) &sock;
//        unix->sun_family = AF_UNIX;
//        strcpy(unix->sun_path, resource->path);
//        sock_len = (int)sizeof(struct sockaddr_un);
    } else {
        zend_throw_exception(ion_class_entry(ION_InvalidUsageException), "Invalid socket name", 0);
        return;
    }
    php_url_free(resource);
    listener->listener = evconnlistener_new_bind(ION(base), _ion_listener_accept, listener,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_EXEC, -1,
                                       &sock, sock_len);
    if(listener->listener) {
        int type = pion_net_sock_name(evconnlistener_get_fd(listener->listener), PION_NET_NAME_LOCAL, &listener->name);
        if(type == PION_NET_NAME_IPV6) {
            listener->flags |= ION_STREAM_NAME_IPV6;
        } else if(type == PION_NET_NAME_UNIX) {
            listener->flags |= ION_STREAM_NAME_UNIX;
        } else if(type == PION_NET_NAME_UNKNOWN || type == FAILURE) {
            /* skip */
        } else {
            listener->flags |= ION_STREAM_NAME_IPV4;
        }
        evconnlistener_enable(listener->listener);
        evconnlistener_set_error_cb(listener->listener, _ion_listener_error);
        listener->flags |= ION_STREAM_STATE_ENABLED;
    } else {
        zend_throw_exception_ex(ion_class_entry(ION_RuntimeException), errno, "Failed to listen on %s: %s", listen->val, evutil_socket_error_to_string(errno));
        return;
    }

}

METHOD_ARGS_BEGIN(ION_Listener, __construct, 2)
    METHOD_ARG_STRING(listen, 0)
    METHOD_ARG_LONG(back_log, 0)
METHOD_ARGS_END();

/** public function ION\Listener::onConnect(callable $action) : ION\Sequence */
CLASS_METHOD(ION_Listener, onConnect) {
    zval * action = NULL;
    ion_listener * listener = get_this_instance(ion_listener);

    if(listener->on_connect) {
        zend_throw_exception(ion_class_entry(ION_InvalidUsageException), "Sequence already defined", 0);
        return;
    }

    ZEND_PARSE_PARAMETERS_START(1, 1)
       Z_PARAM_ZVAL(action)
    ZEND_PARSE_PARAMETERS_END();

    listener->on_connect = ion_promisor_sequence_new(action);

    RETURN_OBJ_ADDREF(listener->on_connect);
}

METHOD_ARGS_BEGIN(ION_Listener, onConnect, 0)
    METHOD_ARG_CALLBACK(action, 0, 0)
METHOD_ARGS_END();

/** public function ION\Listener::enable() : self */
CLASS_METHOD(ION_Listener, enable) {
    ion_listener * listener = get_this_instance(ion_listener);
    if(listener->listener && !(listener->flags & ION_STREAM_STATE_ENABLED)) {
        evconnlistener_enable(listener->listener);
        listener->flags &= ~ION_STREAM_STATE_ENABLED;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Listener, enable);

/** public function ION\Listener::disable() : self */
CLASS_METHOD(ION_Listener, disable) {
    ion_listener * listener = get_this_instance(ion_listener);
    if(listener->listener && (listener->flags & ION_STREAM_STATE_ENABLED)) {
        evconnlistener_disable(listener->listener);
        listener->flags |= ION_STREAM_STATE_ENABLED;
    }
    RETURN_THIS();
}

METHOD_WITHOUT_ARGS(ION_Listener, disable);

/** public function ION\Listener::shutdown() : self */
CLASS_METHOD(ION_Listener, shutdown) {
    ion_listener_release(Z_OBJ_P(getThis()));
}

METHOD_WITHOUT_ARGS(ION_Listener, shutdown);

/** public function ION\Listener::__toString() : string */
CLASS_METHOD(ION_Listener, __toString) {
    ion_listener * listener = get_this_instance(ion_listener);
    RETURN_STR(zend_string_copy(listener->name));
}

METHOD_WITHOUT_ARGS(ION_Listener, __toString);


CLASS_METHODS_START(ION_Listener)
    METHOD(ION_Listener, __construct,   ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, onConnect,     ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, enable,        ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, disable,       ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, shutdown,      ZEND_ACC_PUBLIC)
    METHOD(ION_Listener, __toString,    ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Listener) {
    pion_register_class(ION_Listener, "ION\\Listener", ion_listener_init, CLASS_METHODS(ION_Listener));
    pion_init_std_object_handlers(ION_Listener);
    pion_set_object_handler(ION_Listener, free_obj, ion_listener_free);
    pion_set_object_handler(ION_Listener, clone_obj, NULL);
    return SUCCESS;
}
