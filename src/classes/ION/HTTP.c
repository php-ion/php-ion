#include "ion.h"

zend_class_entry * ion_ce_ION_HTTP;
zend_object_handlers ion_oh_ION_HTTP;

pion_cb * stream_read_token = NULL;

zval ion_http_response_incoming(ion_promisor * promisor, zval * data) {
    zval res;
    ZVAL_UNDEF(&res);

    ZEND_ASSERT(0 && "todo http response");
    return res;
}

/** public function ION\HTTP::request(ION\HTTP\Request $request, ION\Stream $stream, array $options = null) : ION\Deferred */
CLASS_METHOD(ION_HTTP, request) {
    zval          * request = NULL;
    zval          * connect = NULL;
    zend_array    * options = NULL;
    zend_string   * req     = NULL;
    ion_stream    * stream  = NULL;
    zval            token;
    zval            mode;
    zval            length;
    zval            result;
    pion_cb       * stream_read_token = pion_cb_fetch_method("ION\\Stream", "readLine");

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_ZVAL(request)
        Z_PARAM_ZVAL(connect)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_HT(options)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);


    stream = ION_ZOBJ_OBJECT(connect, ion_stream);
    if(stream->state & ION_STREAM_STATE_CLOSED) {
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_HTTP_REQUEST_INVALID_STREAM, 0);
        return;
    }
    if(stream->read) {
        zend_throw_exception(ion_ce_InvalidArgumentException, "Stream already busy", 0);
        return;
    }
    req = pion_http_message_build(Z_OBJ_P(request));
    if(ion_stream_write(stream, req) == false) {
        zend_throw_exception(ion_ce_ION_StreamException, ERR_ION_STREAM_WRITE_FAILED, 0);
        return;
    }
    zend_string_release(req);
    ZVAL_STRING(&token, "\r\n\r\n");
    ZVAL_LONG(&mode, ION_STREAM_MODE_TRIM_TOKEN);
    ZVAL_LONG(&length, 8192);

    result = pion_cb_call_with_3_args(stream_read_token, &token, &mode, &length);
    pion_cb_free(stream_read_token);
    if(!Z_ISUNDEF(result)) {
        return;
    }
    if(Z_TYPE(result) == IS_STRING) {
        zend_object * resp = pion_http_parse_response(Z_STR(result), ion_ce_ION_HTTP_Response);
        RETVAL_OBJ(resp);
        zval_ptr_dtor(&result);
    } else if(Z_TYPE(result) == IS_OBJECT) {
//        ion_promisor_set_internal_done(ION_ZVAL_OBJECT_P(result), ion_http_response_incoming); // ????????
        RETVAL_OBJ(Z_OBJ(result));
    }
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
