#include "../pion.h"
#include "Server.h"

zend_object_handlers ion_oh_ION_Server;
zend_class_entry * ion_ce_ION_Server;

zend_object_handlers ion_oh_ION_ServerException;
zend_class_entry * ion_ce_ION_ServerException;


zend_object * ion_server_init(zend_class_entry * ce) {
    ion_server * server = ecalloc(1, sizeof(ion_server));
    ALLOC_HASHTABLE(server->listeners);
    zend_hash_init(server->listeners, 8, NULL, _zval_dtor_wrapper, 0);
    ALLOC_HASHTABLE(server->conns);
    zend_hash_init(server->conns, 1024, NULL, _zval_dtor_wrapper, 0);

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
    if(server->accept) {
        zend_object_release(server->accept);
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
    if(server->rlimits) {
        bufferevent_rate_limit_group_free(server->rlimits);
    }
}


/** public function ION\Server::listen(string $listen, int $back_log = -1) : Listener */
CLASS_METHOD(ION_Server, listen) {
    zval * host = NULL;
    zval * backlog = NULL;
    ion_server * server = get_this_instance(ion_server);
    zend_object * listener;
    zval          container;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(host)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL_EX(backlog, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(backlog) {
        listener = pion_new_object_arg_2(ion_ce_ION_Listener, host, backlog);
    } else {
        listener = pion_new_object_arg_1(ion_ce_ION_Listener, host);
    }
    if(!listener) {
        return;
    }
    ZVAL_OBJ(&container, listener);
    zend_hash_str_add(server->listeners, Z_STRVAL_P(host), Z_STRLEN_P(host), &container);

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

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Server, setMaxConnsLimit, 0)
    METHOD_ARG_STRING(host, 0)
METHOD_ARGS_END();

CLASS_METHODS_START(ION_Server)
    METHOD(ION_Server, listen,           ZEND_ACC_PUBLIC)
    METHOD(ION_Server, shutdown,         ZEND_ACC_PUBLIC)
    METHOD(ION_Server, setMaxConnsLimit, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Server) {
    pion_register_class(ION_Server, "ION\\Server", ion_server_init, CLASS_METHODS(ION_Server));
    pion_init_std_object_handlers(ION_Server);
    pion_set_object_handler(ION_Server, free_obj, ion_server_free);
    pion_set_object_handler(ION_Server, clone_obj, NULL);

    PION_REGISTER_VOID_EXTENDED_CLASS(ION_ServerException, ion_ce_ION_RuntimeException, "ION\\ServerException");

    return SUCCESS;
}
