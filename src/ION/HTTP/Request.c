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

/** public function ION\HTTP\Request::__toString() : ION\URI */
CLASS_METHOD(ION_HTTP_Request, __toString) {
//    ion_http_message * message = get_this_instance(ion_http_message);

}

METHOD_WITHOUT_ARGS(ION_HTTP_Request, __toString)

CLASS_METHODS_START(ION_HTTP_Request)
    METHOD(ION_HTTP_Request, parse,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Request, getURI,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, getMethod,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Request, __toString,  ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Request) {
    pion_register_extended_class(ION_HTTP_Request, ion_ce_ION_HTTP_Message, "ION\\HTTP\\Request", ion_http_message_init, CLASS_METHODS(ION_HTTP_Request));

    pion_init_std_object_handlers(ION_HTTP_Request);
    pion_set_object_handler(ION_HTTP_Request, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Request, clone_obj, NULL);

    return SUCCESS;
}