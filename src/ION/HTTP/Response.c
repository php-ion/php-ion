#include "../../pion.h"


zend_object_handlers ion_oh_ION_HTTP_Response;
zend_class_entry * ion_ce_ION_HTTP_Response;

zend_object * ion_http_response_init(zend_class_entry * ce) {
    ion_http_message * message = ecalloc(1, sizeof(ion_http_message));
    message->type = ion_http_type_response;
    ALLOC_HASHTABLE(message->headers);
    zend_hash_init(message->headers, 8, NULL, ZVAL_PTR_DTOR, 0);
    RETURN_INSTANCE(ION_HTTP_Response, message);
}

/** public static function ION\HTTP\Response::parse() : self */
CLASS_METHOD(ION_HTTP_Response, parse) {
    zend_string * response_string = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(response_string)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    RETURN_OBJ(pion_http_parse_response(response_string, zend_get_called_scope(execute_data)));
}

METHOD_ARGS_BEGIN(ION_HTTP_Response, parse, 1)
    METHOD_ARG_STRING(response, 0)
METHOD_ARGS_END();

/** public static function ION\HTTP\Response::withStatus(int $code, $reason_phrase = '') : self */
CLASS_METHOD(ION_HTTP_Response, withStatus) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_long          status = 0;
    zend_string      * reason = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(status)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(reason)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(!pion_http_reason((uint8_t)status)) {
        // todo throw exception
        return;
    }

    message->code = (uint8_t)status;

    if(reason && ZSTR_LEN(reason)) {
        message->reason = zend_string_copy(reason);
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_Response, withStatus, 1)
    METHOD_ARG_STRING(response, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Response::getStatusCode() : int */
CLASS_METHOD(ION_HTTP_Response, getStatusCode) {
    ion_http_message * message = get_this_instance(ion_http_message);

    RETURN_LONG(message->code);
}

METHOD_WITHOUT_ARGS(ION_HTTP_Response, getStatusCode)

/** public function ION\HTTP\Response::getReasonPhrase() : string */
CLASS_METHOD(ION_HTTP_Response, getReasonPhrase) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * default_reason = NULL;

    if(message->reason) {
        RETURN_STR(zend_string_copy(message->reason));
    } else if((default_reason = pion_http_reason((uint8_t)message->code))) {
        RETURN_STR(default_reason);
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_Response, getReasonPhrase)

/** public function ION\HTTP\Response::__toString() : string */
CLASS_METHOD(ION_HTTP_Response, __toString) {
    RETURN_STR(pion_http_message_build(Z_OBJ_P(getThis())));
}

METHOD_WITHOUT_ARGS(ION_HTTP_Response, __toString)

CLASS_METHODS_START(ION_HTTP_Response)
    METHOD(ION_HTTP_Response, parse,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Response, withStatus,    ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Response, getStatusCode, ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Response, __toString,    ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Response) {
    pion_register_extended_class(ION_HTTP_Response, ion_ce_ION_HTTP_Message, "ION\\HTTP\\Response", ion_http_response_init, CLASS_METHODS(ION_HTTP_Response));

    pion_init_std_object_handlers(ION_HTTP_Response);
    pion_set_object_handler(ION_HTTP_Response, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Response, clone_obj, NULL);

    return SUCCESS;
}