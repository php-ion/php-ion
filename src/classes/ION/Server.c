#include "../pion.h"
#include "Server.h"

zend_object_handlers ion_oh_ION_Server;
zend_class_entry * ion_ce_ION_Server;

zend_object_handlers ion_oh_ION_ServerException;
zend_class_entry * ion_ce_ION_ServerException;

#define ION_SERVER_ENABLED           (1<<0)
#define ION_SERVER_MAX_CONN_EXCEEDED (1<<1)


static void ion_server_connect_handler(zend_object * connect) {
    ion_stream   * stream = get_object_instance(connect, ion_stream);
    ion_server   * server = stream->storage;
    zend_string  * name = NULL;
    zval           connect_container;

    server->total_conns++;
    stream->storage = server;
    if(server->priority >= 0) {
        ion_stream_set_priority(stream, server->priority);
    }
    if(server->group) {
        ion_stream_set_group(stream, server->group);
    }
    if(server->input_buffer_size) {
        ion_stream_set_input_size(stream, stream->input_size);
    }
    name = ion_stream_get_name_remote(connect);
    if(!name) {
        return;
    }
    ZVAL_OBJ(&connect_container, connect);
    zval_add_ref(&connect_container);
    zend_hash_str_add_new(server->conns, name->val, name->len, &connect_container);
    zend_string_release(name);
    if(server->connect_handler) {
        ion_promisor_sequence_invoke(server->connect, &connect_container);
    } else {
        // todo: autofree
    }
    return;
}

static void ion_server_incoming_handler(zend_object * connect) {
    ion_stream   * stream = get_object_instance(connect, ion_stream);
    ion_server   * server = stream->storage;

    server->total_resumed++;

    if(server->incoming) {
        zval connect_container;
        ZVAL_OBJ(&connect_container, connect);
        ion_promisor_sequence_invoke(server->incoming, &connect_container);
    } else {
        // todo: autofree
    }
}

static void ion_server_free_handler(zend_object * connect) {
    ion_stream   * stream = get_object_instance(connect, ion_stream);
    ion_server   * server = stream->storage;

    server->total_resumed++;

    ion_stream_suspend(stream);

    if(server->free_handler) {
        zval connect_container;
        ZVAL_OBJ(&connect_container, connect);
        ion_promisor_sequence_invoke(server->incoming, &connect_container);
    } else {
        // todo: autofree
    }
}

static void ion_server_close_handler(zend_object * connect) {
    ion_stream   * stream = get_object_instance(connect, ion_stream);
    ion_server   * server = stream->storage;
    zend_string  * name = ion_stream_get_name_remote(connect);
    if(!name) {
        return;
    }
    zend_hash_str_del(server->conns, name->val, name->len);
    zend_string_release(name);
}

static void ion_server_timeout_handler(zend_object * connect) {
    ion_stream   * stream = get_object_instance(connect, ion_stream);
    ion_server   * server = stream->storage;
    if(server->close) {
        zval connect_container;
        ZVAL_OBJ(&connect_container, connect);
        ion_promisor_sequence_invoke(server->close, &connect_container);
    } else {
        ion_stream_close_fd(stream);
    }
}

static void ion_server_ping_hanler(zend_object * connect) {
    ion_stream   * stream = get_object_instance(connect, ion_stream);
    ion_server   * server = stream->storage;
    if(server->ping) {
        zval connect_container;
        ZVAL_OBJ(&connect_container, connect);
        ion_promisor_sequence_invoke(server->ping, &connect_container);
    }
}

zend_object * ion_server_init(zend_class_entry * ce) {
    ion_server * server = ecalloc(1, sizeof(ion_server));

    ALLOC_HASHTABLE(server->listeners);
    zend_hash_init(server->listeners, 8, NULL, _zval_dtor_wrapper, 0);
    ALLOC_HASHTABLE(server->conns);
    zend_hash_init(server->conns, 1024, NULL, _zval_dtor_wrapper, 0);
    server->connect = ion_promisor_sequence_new(NULL);

    server->connect_handler  = ion_server_connect_handler;
    server->incoming_handler = ion_server_incoming_handler;
    server->free_handler     = ion_server_free_handler;
    server->close_handler    = ion_server_close_handler;
    server->timeout_handler  = ion_server_timeout_handler;
    server->ping_handler     = ion_server_ping_hanler;
    RETURN_INSTANCE(ION_Server, server);
}

void ion_server_free(zend_object * object) {
    ion_server * server = get_object_instance(object, ion_server);
    zend_object_std_dtor(object);
    zend_hash_clean(server->listeners);
    zend_hash_destroy(server->listeners);
    FREE_HASHTABLE(server->listeners);
    zend_hash_clean(server->conns);
    zend_hash_destroy(server->conns);
    FREE_HASHTABLE(server->conns);
    if(server->connect) {
        zend_object_release(server->connect);
    }
    if(server->incoming) {
        zend_object_release(server->incoming);
    }
    if(server->timeout) {
        zend_object_release(server->timeout);
    }
    if(server->close) {
        zend_object_release(server->close);
    }
    if(server->group) {
        bufferevent_rate_limit_group_free(server->group);
    }
}

void ion_server_enable(zend_object * server_obj, zend_bool state) {
    ion_server * server = get_object_instance(server_obj, ion_server);
    zval       * listener = NULL;

    ZEND_HASH_FOREACH_VAL(server->listeners, listener) {
        ion_listener_enable(Z_OBJ_P(listener), state);
    } ZEND_HASH_FOREACH_END();
}

/** public function ION\Server::listen(string $listen, int $back_log = -1) : Listener */
CLASS_METHOD(ION_Server, listen) {
    zval         * host = NULL;
    zval         * backlog = NULL;
    ion_server   * server = get_this_instance(ion_server);
    zend_object  * listener_object;
    ion_listener * listener;
    zval           container;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(host)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(backlog, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(backlog) {
        listener_object = pion_new_object_arg_2(ion_ce_ION_Listener, host, backlog);
    } else {
        listener_object = pion_new_object_arg_1(ion_ce_ION_Listener, host);
    }
    if(!listener_object) {
        return;
    }

    listener = get_object_instance(listener_object, ion_listener);
    listener->storage = server;

    ZVAL_OBJ(&container, listener_object);
    zend_hash_str_add(server->listeners, listener->name->val, listener->name->len, &container);

    RETURN_ZVAL(&container, 1, 0);
}

METHOD_ARGS_BEGIN(ION_Server, listen, 2)
    METHOD_ARG(host, 0)
    METHOD_ARG(back_log, 0)
METHOD_ARGS_END();


/** public function ION\Server::shutdown(string $host) : Listener */
CLASS_METHOD(ION_Server, shutdown) {
    zval * host = NULL;
    ion_server * server = get_this_instance(ion_server);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(host)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    zend_hash_str_del(server->listeners, Z_STRVAL_P(host), Z_STRLEN_P(host));

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Server, shutdown, 0)
    METHOD_ARG_STRING(host, 0)
METHOD_ARGS_END();

/** public function ION\Server::setMaxConnsLimit(int $max) : self  */
CLASS_METHOD(ION_Server, setMaxConnsLimit) {
    zend_long max = -1;
    ion_server * server = get_this_instance(ion_server);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(max)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);


    server->max_conns = max;
    if(server->current_conns < max || max < 0) {
        if(server->flags & ION_SERVER_MAX_CONN_EXCEEDED) {
            server->flags &= ~ION_SERVER_MAX_CONN_EXCEEDED;
            if(server->flags & ION_SERVER_ENABLED) {
                ion_server_enable(&server->std, true);
            }
        }

    } else if(server->current_conns > max) {
        if(!(server->flags & ION_SERVER_MAX_CONN_EXCEEDED)) {
            server->flags |= ION_SERVER_MAX_CONN_EXCEEDED;
            if(server->flags & ION_SERVER_ENABLED) {
                ion_server_enable(&server->std, false);
            }
        }
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Server, setMaxConnsLimit, 0)
    METHOD_ARG_LONG(max, 0)
METHOD_ARGS_END();

/** public function ION\Server::accept() : Sequence  */
CLASS_METHOD(ION_Server, accept) {
    ion_server * server = get_this_instance(ion_server);

    zend_object_addref(server->connect);
    RETURN_OBJ(server->connect);
}

METHOD_WITHOUT_ARGS(ION_Server, accept)

CLASS_METHODS_START(ION_Server)
    METHOD(ION_Server, listen,           ZEND_ACC_PUBLIC)
    METHOD(ION_Server, shutdown,         ZEND_ACC_PUBLIC)
    METHOD(ION_Server, setMaxConnsLimit, ZEND_ACC_PUBLIC)
    METHOD(ION_Server, accept,           ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Server) {
    pion_register_class(ION_Server, "ION\\Server", ion_server_init, CLASS_METHODS(ION_Server));
    pion_init_std_object_handlers(ION_Server);
    pion_set_object_handler(ION_Server, free_obj, ion_server_free);
    pion_set_object_handler(ION_Server, clone_obj, NULL);

    PION_REGISTER_VOID_EXTENDED_CLASS(ION_ServerException, ion_ce_ION_RuntimeException, "ION\\ServerException");

    return SUCCESS;
}
