#include "ion.h"

zend_class_entry * ion_ce_ION_HTTP;
zend_object_handlers ion_oh_ION_HTTP;

/** public function ION\HTTP::request(ION\HTTP\Request $request, ION\Stream $stream, array $options = null) : ION\HTTP\Response */
CLASS_METHOD(ION_HTTP, request) {
    zval          * request = NULL;
    zval          * connect = NULL;
    zend_array    * options = NULL;
    zend_string   * req     = NULL;
    ion_stream    * stream  = NULL;
    ion_stream_token token;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_ZVAL(request)
        Z_PARAM_ZVAL(connect)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_HT(options)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);


//    stream = get_object_instance(connect, ion_stream);
//    // todo: check stream
//    req = pion_http_message_build(Z_OBJ_P(connect));
//    // todo: check req
//    bufferevent_write(stream->buffer, req->val, req->len);
//    stream->state &= ~ION_STREAM_STATE_FLUSHED;
//    token.token = ION_STR(ION_STR_CRLFCRLF);
}

METHOD_ARGS_BEGIN(ION_HTTP, request, 2)
    METHOD_ARG_OBJECT(request, ION\\HTTP\\Request, 0, 0)
    METHOD_ARG_OBJECT(connect, ION\\Stream, 0, 0)
    METHOD_ARG_ARRAY(options, 0, 1)
METHOD_ARGS_END();

/** public function ION\HTTP::getResponseReason(int $response_code) : ION\HTTP\Request */
CLASS_METHOD(ION_HTTP, getResponseReason) {
    zend_long code = 0;
    zend_string * reason;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    reason = pion_http_reason((uint16_t)code);
    if(reason) {
        RETURN_STR(reason);
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
