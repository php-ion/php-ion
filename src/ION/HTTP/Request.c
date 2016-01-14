#include "../../pion.h"


zend_object_handlers ion_oh_ION_HTTP_Request;
zend_class_entry * ion_ce_ION_HTTP_Request;


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
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * request_string;
    zend_string      * head_string;
    zend_string      * target = NULL;
    zend_string      * header = NULL;
    zval             * value = NULL;

    if(message->target) {
        target = zend_string_copy(message->target);
    } else if(message->uri) {
        target = ion_uri_stringify(message->uri, URI_ALL);
    }

    head_string = strpprintf(
        1000,
        "%s %s HTTP/%s\r\n",
        message->method ? message->method->val : "GET",
        target ? target->val : "/",
        message->version ? message->version->val : "1.1"
    );
    zend_string_release(target);

    ZEND_HASH_FOREACH_STR_KEY_VAL(message->headers, header, value) {
        zend_bool to_up = true;
        for(size_t p = 0; p < header->len; p++) {

        }
        if(zend_hash_num_elements(Z_ARR_P(value)) == 1) {

        } else {

        }
    } ZEND_HASH_FOREACH_END();

}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, __toString)

CLASS_METHODS_START(ION_HTTP_Request)
    METHOD(ION_HTTP_Request, parse,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Request, getURI,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getMethod,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, withRequestTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getRequestTarget,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, __toString,  ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Request) {
    pion_register_extended_class(ION_HTTP_Request, ion_ce_ION_HTTP_Message, "ION\\HTTP\\Request", ion_http_message_init, CLASS_METHODS(ION_HTTP_Request));

    pion_init_std_object_handlers(ION_HTTP_Request);
    pion_set_object_handler(ION_HTTP_Request, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Request, clone_obj, NULL);

    return SUCCESS;
}