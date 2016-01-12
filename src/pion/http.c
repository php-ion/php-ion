#include "../pion.h"

int pion_http_message_begin(http_parser * parser) {
    return 0;
}

int pion_http_url(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    return 0;
}

int pion_http_header_name(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    return 0;
}

int pion_http_header_value(http_parser * parser, const char * at, size_t length) {
    return 0;
}

int pion_http_message_complete(http_parser * parser) {
    ion_http_message * message = parser->data;
    message->flags |= ION_HTTP_MESSAGE_COMPLATE;
    return 0;
}


zend_object * pion_http_parse_request(zend_string * request_string, zend_class_entry * ce) {
    zend_object          * request = pion_new_object_arg_0(ce ? ce : ion_ce_ION_HTTP_Request);
    ion_http_message     * message = get_object_instance(request, ion_http_message);
    http_parser_settings   settings;
    size_t                 nparsed;


    message->parser = ecalloc(1, sizeof(http_parser));
    http_parser_init(message->parser, HTTP_REQUEST);

    message->parser->data = message;
    memset(&settings, 0, sizeof(settings));
    settings.on_message_complete = pion_http_message_begin;
    settings.on_url = pion_http_url;
    settings.on_header_field = pion_http_header_name;
    settings.on_header_value = pion_http_header_value;
    settings.on_message_complete = pion_http_message_complete;

    nparsed = http_parser_execute(message->parser, &settings, request_string->val, request_string->len);
    if(nparsed < request_string->len) {
        zend_error(E_NOTICE, "has unparsed data");
    }
    return request;
}


const char * pion_http_reason(uint16_t response_code) {
    switch(response_code) {
        // 1xx
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocols";
        case 102:
            return "Processing";
        // 2xx
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 207:
            return "Multi-Status";
        case 226:
            return "IM Used";
        // 3xx
        case 300:
            return "Multiple Choices";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Moved Temporarily";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 305:
            return "Use Proxy";
        case 307:
            return "Temporary Redirect";
        // 4xx
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 402:
            return "Payment Required";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Request Entity Too Large";
        case 414:
            return "Request-URI Too Large";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 418:
            return "I'm a teapot";
        case 422:
            return "Unprocessable Entity";
        case 423:
            return "Locked";
        case 424:
            return "Failed Dependency";
        case 425:
            return "Unordered Collection";
        case 426:
            return "Upgrade Required";
        case 428:
            return "Precondition Required";
        case 429:
            return "Too Many Requests";
        case 431:
            return "Request Header Fields Too Large";
        case 434:
            return "Requested host unavailable";
        case 449:
            return "Retry With";
        case 451:
            return "Unavailable For Legal Reasons";
        // 5xx
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        case 506:
            return "Variant Also Negotiates";
        case 507:
            return "Insufficient Storage";
        case 508:
            return "Loop Detected";
        case 509:
            return "Bandwidth Limit Exceeded";
        case 510:
            return "Not Extended";
        case 511:
            return "Network Authentication Required";
        default:
            return NULL;
    }
}