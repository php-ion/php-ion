#include "../pion.h"
#include "HTTP/Message.h"
#include "HTTP.h"
#include "../external/http-parser/http_parser.h"

zend_class_entry * ion_ce_ION_HTTP;
zend_object_handlers ion_oh_ION_HTTP;

int _ion_http_header_name(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    return 1;
}

int _ion_http_header_value(http_parser * parser, const char * at, size_t length) {
    return 1;
}


/** public function ION\HTTP::parseRequest(string $request) : ION\HTTP\Request */
CLASS_METHOD(ION_HTTP, parseRequest) {
    zend_string          * request_data = NULL;
    zend_object          * request = NULL;
    ion_http_message     * message = NULL;
    http_parser_settings   settings;
    http_parser          * parser = NULL;
    size_t                 nparsed;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(request_data)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    request = pion_new_object_arg_0(ion_ce_ION_HTTP_Message);
    message = get_object_instance(request, ion_http_message);

    settings.on_header_field = _ion_http_header_name;
    settings.on_header_value = _ion_http_header_value;

    parser = emalloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    parser->data = message;

    nparsed = http_parser_execute(parser, &settings, request_data->val, 0);
//    parser->
}

METHOD_ARGS_BEGIN(ION_HTTP, parseRequest, 1)
    METHOD_ARG_STRING(request, 0)
METHOD_ARGS_END();

CLASS_METHODS_START(ION_HTTP)
//    METHOD(ION_HTTP, parseHeaders,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP, parseRequest,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
//    METHOD(ION_HTTP, parseResponse, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_HTTP) {
    PION_REGISTER_STATIC_CLASS(ION_HTTP, "ION\\HTTP");

    return SUCCESS;
}