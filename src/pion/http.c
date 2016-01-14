#include "../pion.h"

zend_string * pion_http_buffering(zend_string * buffer, const char * at, size_t length) {
    if(buffer) {
        size_t offset = buffer->len;
        zend_string_addref(buffer);
        zend_string * new_buffer = zend_string_extend(buffer, buffer->len + length, 0);
        memcpy(&new_buffer->val[offset], at, length);
        new_buffer->val[new_buffer->len] = '\000';
        zend_string_release(buffer);
        return new_buffer;
    } else {
        return zend_string_init(at, length, 0);
    }
}

int pion_http_message_begin(http_parser * parser) {

    return 0;
}

int pion_http_url(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    if(length == 1 && at[0] == '/') {
        message->uri = ion_uri_parse(ION_STR(ION_STR_SLASH));
    } else {
        zend_string * uri = zend_string_init(at, length, 0);
        message->uri = ion_uri_parse(uri);
        zend_string_release(uri);
    }
    message->method = ION_HTTP_METHOD_STR(parser->method);
    return 0;
}

int pion_http_header_name(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    message->parser->buffer = zend_string_init(at, length, 0);
    return 0;
}

int pion_http_header_value(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    zval             * header = NULL;
    zval               value;
    zend_string      * name_lower;
    if(!message->parser->buffer) {
        zend_error(E_NOTICE, "Unexpected HTTP parser behavior: header value without header name");
        return 1;
    }

    name_lower = zend_string_tolower(message->parser->buffer);

    header = zend_hash_find(message->headers, name_lower);
    if(!header) {
        zval h;
        array_init(&h);
        header = zend_hash_add(message->headers, name_lower, &h);
    }
    ZVAL_STR(&value, zend_string_init(at, length, 0));
    zend_hash_next_index_insert(Z_ARR_P(header), &value);

    zend_string_release(name_lower);
    zend_string_release(message->parser->buffer);
    message->parser->buffer = NULL;

    return 0;
}

int pion_http_message_body(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    message->body = pion_http_buffering(message->body, at, length);
    return 0;
}

//int pion_http_message_chunk_header(http_parser * parser) {
//    ion_http_message * message = parser->data;
//
//    return 0;
//}
//
//int pion_http_message_chunk_complete(http_parser * parser) {
//    ion_http_message * message = parser->data;
//
//    return 0;
//}


int pion_http_headers_complete(http_parser * parser) {
    ion_http_message * message = parser->data;
    zval             * header = NULL;

    header = zend_hash_find(message->headers, ION_STR(ION_STR_CONTENT_TYPE));
    if(header) {
        zval * value = zend_hash_get_current_data(message->headers);
        zend_string * v = Z_STR_P(value);
        if(strstr(v->val, "multipart/form-data")) {
            char * boundary_ptr = strstr(v->val, "boundary=");
            if(boundary_ptr) {
                boundary_ptr += sizeof("boundary=") - 1;
                size_t start = boundary_ptr - v->val;
                for(size_t end = start; end < v->len; end++) {
                    if(isalpha(v->val[end]) || isdigit(v->val[end]) || v->val[end]=='_' || v->val[end] == '-') {

                    }
                }
            }
        }
    }
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
    http_parser          * parser;
    multipart_parser_settings mp_settings;


    message->parser = ecalloc(1, sizeof(ion_http_parser));
    parser = ecalloc(1, sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    message->parser->parser = parser;

    parser->data = message;
//    memset(&mp_settings, 0, sizeof(mp_settings));
    memset(&settings, 0, sizeof(settings));
    settings.on_message_complete = pion_http_message_begin;
    settings.on_url              = pion_http_url;
    settings.on_header_field     = pion_http_header_name;
    settings.on_header_value     = pion_http_header_value;
    settings.on_message_complete = pion_http_message_complete;
    settings.on_body             = pion_http_message_body;
    settings.on_headers_complete = pion_http_headers_complete;
//    mp_settings.
//    settings.on_chunk_header     = pion_http_message_chunk_header;
//    settings.on_chunk_complete   = pion_http_message_chunk_complete;

    nparsed = http_parser_execute(parser, &settings, request_string->val, request_string->len);
    if(nparsed < request_string->len) {
        zend_error(E_NOTICE, "has unparsed data");
    }
    if(parser->http_major == 1 && parser->http_minor == 1) {
        message->version = ION_STR(ION_STR_V11);
    } else if (parser->http_major == 1 && parser->http_minor == 0) {
        message->version = ION_STR(ION_STR_V10);
    } else if (parser->http_major == 2 && parser->http_minor == 0) {
        message->version = ION_STR(ION_STR_V20);
    } else if (parser->http_major == 0 && parser->http_minor == 9) {
        message->version = ION_STR(ION_STR_V09);
    } else {
        message->version = strpprintf(10, "%d.%d", parser->http_major, parser->http_minor);
    }
    efree(parser);
    efree(message->parser);
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