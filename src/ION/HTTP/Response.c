#include "../../pion.h"


zend_object_handlers ion_oh_ION_HTTP_Response;
zend_class_entry * ion_ce_ION_HTTP_Response;

#define ION_HTTP_RESPONSE_VERSION 1
#define ION_HTTP_RESPONSE_STATUS  2
#define ION_HTTP_RESPONSE_REASON  3
#define ION_HTTP_RESPONSE_HEADERS 4
#define ION_HTTP_RESPONSE_BODY    5

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

/** public static function ION\HTTP\Response::factory(array $options = []) : static */
CLASS_METHOD(ION_HTTP_Response, factory) {
    zend_array       * options = NULL;
    zend_object      * request;
    ion_http_message * message;
    zend_long          opt;
    zval             * option = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ARRAY_HT(options)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    request = pion_new_object_arg_0(zend_get_called_scope(execute_data));
    if(!request) {
        return;
    }
    message = get_object_instance(request, ion_http_message);
    if(options) {
        ZEND_HASH_FOREACH_NUM_KEY_VAL(options, opt, option) {
            switch (opt) {
                case ION_HTTP_RESPONSE_STATUS:
                    if (Z_TYPE_P(option) != IS_LONG) {
                        zend_throw_exception(ion_ce_InvalidArgumentException,
                                             ERR_ION_HTTP_RESPONSE_FACTORY_STATUS, 0);
                        return;
                    }
                    if (!pion_http_reason((uint16_t) Z_LVAL_P(option))) {
                        zend_throw_exception_ex(ion_ce_InvalidArgumentException, 0,
                                                ERR_ION_HTTP_RESPONSE_UNKNOWN_STATUS, Z_LVAL_P(option));
                        return;
                    }
                    if (message->reason) {
                        zend_string_release(message->reason);
                        message->reason = NULL;
                    }
                    message->status = (uint16_t) Z_LVAL_P(option);
                    break;
                case ION_HTTP_RESPONSE_REASON:
                    zval_add_ref(option);
                    if (Z_TYPE_P(option) != IS_STRING) {
                        convert_to_string(option);
                    }
                    if (message->reason) {
                        zend_string_release(message->reason);
                    }
                    message->reason = Z_STR_P(option);
                    break;
                case ION_HTTP_RESPONSE_VERSION:
                    zval_add_ref(option);
                    if (Z_TYPE_P(option) != IS_STRING) {
                        convert_to_string(option);
                    }
                    if (message->version) {
                        zend_string_release(message->version);
                    }
                    message->version = Z_STR_P(option);
                    break;
                case ION_HTTP_RESPONSE_HEADERS:
                    if (Z_TYPE_P(option) != IS_ARRAY) {
                        zend_throw_exception(ion_ce_InvalidArgumentException,
                                             ERR_ION_HTTP_REQUEST_FACTORY_HEADERS, 0);
                        return;
                    }
                    if (message->headers) {
                        zend_hash_clean(message->headers);
                    }
                    zend_string * header;
                    zval * value;
                    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARR_P(option), header, value) {
                        ion_http_message_with_added_header(message->headers, header, value);
                    } ZEND_HASH_FOREACH_END();
                    break;
                case ION_HTTP_RESPONSE_BODY:
                    zval_add_ref(option);
                    if (Z_TYPE_P(option) != IS_STRING) {
                        convert_to_string(option);
                    }
                    if (message->body) {
                        zend_string_release(message->body);
                    }
                    message->body = Z_STR_P(option);
                    break;
                default:
                    zend_throw_exception_ex(ion_ce_InvalidArgumentException, 0,
                                            ERR_ION_HTTP_REQUEST_FACTORY_UNKNOWN, opt);
                    return;
            }
        } ZEND_HASH_FOREACH_END();
    }

    RETURN_OBJ(request);
}

METHOD_ARGS_BEGIN(ION_HTTP_Response, factory, 1)
    METHOD_ARG_ARRAY(options, 0, 0)
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

    if(!pion_http_reason((uint16_t)status)) {
        zend_throw_exception_ex(ion_ce_InvalidArgumentException, 0, ERR_ION_HTTP_RESPONSE_UNKNOWN_STATUS, status);
        return;
    }

    message->status = (uint8_t)status;

    if(message->reason){
        zend_string_release(message->reason);
        message->reason = NULL;
    }

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

    RETURN_LONG(message->status);
}

METHOD_WITHOUT_ARGS(ION_HTTP_Response, getStatusCode)

/** public function ION\HTTP\Response::getReasonPhrase() : string */
CLASS_METHOD(ION_HTTP_Response, getReasonPhrase) {
    ion_http_message * message = get_this_instance(ion_http_message);
    zend_string      * default_reason = NULL;

    if(message->reason) {
        RETURN_STR(zend_string_copy(message->reason));
    } else if((default_reason = pion_http_reason((uint8_t)message->status))) {
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
    METHOD(ION_HTTP_Response, parse,           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Response, factory,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_Response, withStatus,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Response, getStatusCode,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Response, getReasonPhrase, ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_Response, __toString,      ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_Response) {
    pion_register_extended_class(ION_HTTP_Response, ion_ce_ION_HTTP_Message, "ION\\HTTP\\Response", ion_http_response_init, CLASS_METHODS(ION_HTTP_Response));

    PION_CLASS_CONST_LONG(ION_HTTP_Response, "VERSION", ION_HTTP_RESPONSE_VERSION);
    PION_CLASS_CONST_LONG(ION_HTTP_Response, "STATUS",  ION_HTTP_RESPONSE_STATUS);
    PION_CLASS_CONST_LONG(ION_HTTP_Response, "REASON",  ION_HTTP_RESPONSE_REASON);
    PION_CLASS_CONST_LONG(ION_HTTP_Response, "HEADERS", ION_HTTP_RESPONSE_HEADERS);
    PION_CLASS_CONST_LONG(ION_HTTP_Response, "BODY",    ION_HTTP_RESPONSE_BODY);

    pion_init_std_object_handlers(ION_HTTP_Response);
    pion_set_object_handler(ION_HTTP_Response, free_obj, ion_http_message_free);
    pion_set_object_handler(ION_HTTP_Response, clone_obj, NULL);

    return SUCCESS;
}