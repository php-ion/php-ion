#include "../pion.h"

int pion_http_header_name(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    return 1;
}

int pion_http_header_value(http_parser * parser, const char * at, size_t length) {
    return 1;
}


zend_object * pion_http_parse_request(zend_string * request_string) {
    zend_object      * request = pion_new_object_arg_0(ion_ce_ION_HTTP_Message);
    ion_http_message * message = get_object_instance(request, ion_http_message);

    return request;
}