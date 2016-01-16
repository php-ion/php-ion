#include "../../pion.h"


zend_object_handlers ion_oh_ION_HTTP_Request;
zend_class_entry * ion_ce_ION_HTTP_Request;


#define ION_HTTP_REQUEST_URI     1
#define ION_HTTP_REQUEST_TARGET  2
#define ION_HTTP_REQUEST_VERSION 3
#define ION_HTTP_REQUEST_METHOD  4
#define ION_HTTP_REQUEST_HEADERS 5
#define ION_HTTP_REQUEST_BODY    6

zend_object * ion_http_request_init(zend_class_entry * ce) {
    ion_http_message * message = ecalloc(1, sizeof(ion_http_message));
    message->type = ion_http_type_request;
    ALLOC_HASHTABLE(message->headers);
    zend_hash_init(message->headers, 8, NULL, ZVAL_PTR_DTOR, 0);
    RETURN_INSTANCE(ION_HTTP_Request, message);
}

CLASS_METHOD(ION_HTTP_Request, parse) {
    zend_string * request_string = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(request_string)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    RETURN_OBJ(pion_http_parse_request(request_string, zend_get_called_scope(execute_data)));
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, parse, 1)
    METHOD_ARG_STRING(request, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Request::factory() : static */
CLASS_METHOD(ION_HTTP_Request, factory) {
    zend_array       * options = NULL;
    zend_object      * request;
    ion_http_message * message;
    zend_long          option;
    zval             * value = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(options)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    request = pion_new_object_arg_0(zend_get_called_scope(execute_data));
    if(!request) {
        return;
    }
    message = get_object_instance(request, ion_http_message);
    ZEND_HASH_FOREACH_NUM_KEY_VAL(options, option, value) {
        switch(option) {
            case ION_HTTP_REQUEST_URI:
                if(Z_TYPE_P(value) != IS_OBJECT || !instanceof_function(Z_CE_P(value), ion_ce_ION_URI)) {
                    zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_HTTP_REQUEST_FACTORY_URI, 0);
                    return;
                }
                if(message->uri) {
                    zend_object_release(message->uri);
                }
                zval_add_ref(value);
                message->uri = Z_OBJ_P(value);
                break;
            case ION_HTTP_REQUEST_TARGET:
                if(Z_TYPE_P(value) == IS_STRING) {
                    zval_add_ref(value);
                } else {
                    convert_to_string(value);
                }
                if(message->target) {
                    zend_string_release(message->target);
                }
                message->target = Z_STR_P(value);
                break;
            case ION_HTTP_REQUEST_VERSION:
                if(Z_TYPE_P(value) == IS_STRING) {
                    zval_add_ref(value);
                } else {
                    convert_to_string(value);
                }
                if(message->version) {
                    zend_string_release(message->version);
                }
                message->version = Z_STR_P(value);
                break;
            case ION_HTTP_REQUEST_METHOD:
                if(Z_TYPE_P(value) != IS_STRING) {
                    zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_HTTP_REQUEST_FACTORY_METHOD, 0);
                    return;
                }
                if(message->method) {
                    zend_string_release(message->method);
                }
                message->method = Z_STR_P(value);
                break;
            case ION_HTTP_REQUEST_HEADERS:
                if(Z_TYPE_P(value) != IS_ARRAY) {
                    zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_HTTP_REQUEST_FACTORY_HEADERS, 0);
                    return;
                }
                if(message->headers) {
                    zend_hash_clean(message->headers);
                }
                zend_array  * headers = zend_array_dup(Z_ARR_P(value));
                zend_string * header;
                zval          copy;
                zval        * v;
                zval        * vv;
                ZEND_HASH_FOREACH_STR_KEY_VAL(headers, header, vv) {
                            ZVAL_COPY(&copy, vv);
                    v = zend_hash_add(message->headers, zend_string_tolower(header), &copy);
                    if(Z_TYPE_P(v) == IS_STRING) {
                        convert_to_array(v);
                    } else if(Z_TYPE_P(v) == IS_ARRAY) {
                        zval * hv;
                        ZEND_HASH_FOREACH_VAL(Z_ARR_P(v), hv) {
                            if(Z_TYPE_P(hv) != IS_STRING) {
                                convert_to_string(hv);
                            }
                        } ZEND_HASH_FOREACH_END();
                    }
                } ZEND_HASH_FOREACH_END();

                message->headers = headers;
                break;
            case ION_HTTP_REQUEST_BODY:
                if(Z_TYPE_P(value) == IS_STRING) {
                    zval_add_ref(value);
                } else {
                    convert_to_string(value);
                }
                if(message->body) {
                    zend_string_release(message->body);
                }
                message->body = Z_STR_P(value);
                break;
            default:
                zend_throw_exception_ex(ion_ce_InvalidArgumentException, 0, ERR_ION_HTTP_REQUEST_FACTORY_UNKNOWN, option);
                return;
        }
    } ZEND_HASH_FOREACH_END();

    RETURN_OBJ(request);
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, factory, 1)
    METHOD_ARG_ARRAY(options, 0, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Request::getURI() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, getURI) {
    ion_http_message * message = get_this_instance(ion_http_message);

    zend_object_addref(message->uri);
    RETURN_OBJ(message->uri);
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, getURI)

/** public function ION\HTTP\Request::getMethod() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, getMethod) {
    ion_http_message * message = get_this_instance(ion_http_message);

    if(message->method) {
        zend_string_addref(message->method);
        RETURN_STR(message->method);
    } else {
        RETURN_STR(ION_STR(ION_STR_UP_GET));
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, getMethod)

/** public function ION\HTTP\Request::withMethod() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, withMethod) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * method = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(method)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(message->method) {
        zend_string_release(message->method);
    }

    message->method = zend_string_copy(method);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, withMethod, 1)
    METHOD_ARG_STRING(method, 0)
METHOD_ARGS_END();


/** public function ION\HTTP\Request::getRequestTarget() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, getRequestTarget) {
    ion_http_message * message = get_this_instance(ion_http_message);

    if(message->target) {
        zend_string_addref(message->target);
        RETURN_STR(message->target);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, getRequestTarget)

/** public function ION\HTTP\Request::withRequestTarget() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, withRequestTarget) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * target = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(message->target) {
        zend_string_release(message->target);
    }

    message->target = zend_string_copy(target);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Request, withRequestTarget, 1)
    METHOD_ARG_STRING(target, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Request::__toString() : string */
CLASS_METHOD(ION_HTTP_Request, __toString) {
    RETURN_STR(pion_http_message_build(Z_OBJ_P(getThis())));
}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, __toString)

CLASS_METHODS_START(ION_HTTP_Request)
    METHOD(ION_HTTP_Request, parse,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Request, factory,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Request, getURI,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getMethod,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, withMethod,  ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, withRequestTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getRequestTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, __toString,  ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Request) {
    pion_register_extended_class(ION_HTTP_Request, ion_ce_ION_HTTP_Message, "ION\\HTTP\\Request", ion_http_request_init, CLASS_METHODS(ION_HTTP_Request));

    PION_CLASS_CONST_LONG(ION_HTTP_Request, "URI",     ION_HTTP_REQUEST_URI);
    PION_CLASS_CONST_LONG(ION_HTTP_Request, "TARGET",  ION_HTTP_REQUEST_TARGET);
    PION_CLASS_CONST_LONG(ION_HTTP_Request, "VERSION", ION_HTTP_REQUEST_VERSION);
    PION_CLASS_CONST_LONG(ION_HTTP_Request, "METHOD",  ION_HTTP_REQUEST_METHOD);
    PION_CLASS_CONST_LONG(ION_HTTP_Request, "HEADERS", ION_HTTP_REQUEST_HEADERS);
    PION_CLASS_CONST_LONG(ION_HTTP_Request, "BODY",    ION_HTTP_REQUEST_BODY);

    pion_init_std_object_handlers(ION_HTTP_Request);
    pion_set_object_handler(ION_HTTP_Request, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Request, clone_obj, NULL);

    return SUCCESS;
}