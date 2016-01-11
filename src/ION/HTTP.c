#include "../pion.h"

zend_class_entry * ion_ce_ION_HTTP;
zend_object_handlers ion_oh_ION_HTTP;


int _ion_http_header_name(http_parser * parser, const char * at, size_t length) {
    return 1;
}

int _ion_http_header_value(http_parser * parser, const char * at, size_t length) {
    return 1;
}


/** public function ION\HTTP::request(ION\HTTP\Request $request, ION\Stream $stream = null) : ION\HTTP\Request */
CLASS_METHOD(ION_HTTP, request) {
    zval          * request = NULL;
    zval          * connect = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(request)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(connect)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
}

METHOD_ARGS_BEGIN(ION_HTTP, request, 1)
    METHOD_ARG_OBJECT(request, ION\\HTTP\\Request, 0, 0)
    METHOD_ARG_OBJECT(connect, ION\\Stream, 1, 0)
METHOD_ARGS_END();

/** public function ION\HTTP::getResponseReason(int $response_code) : ION\HTTP\Request */
CLASS_METHOD(ION_HTTP, getResponseReason) {
    zend_long code = 0;
    const char * reason;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    reason = pion_http_reason((uint16_t)code);
    if(reason) {
        RETURN_STRING(reason);
    } else {
        RETURN_NULL();
    }
}

METHOD_ARGS_BEGIN(ION_HTTP, getResponseReason, 1)
    METHOD_ARG_STRING(response_code, 0)
METHOD_ARGS_END();

CLASS_METHODS_START(ION_HTTP)
    METHOD(ION_HTTP, request,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP, getResponseReason,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP) {

    PION_REGISTER_STATIC_CLASS(ION_HTTP, "ION\\HTTP");
    return SUCCESS;
}