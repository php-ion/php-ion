#include "ion.h"
#include "http.h"


static const char transform_map[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        0,       1,       1,       1,       1,       1,       1,       1,
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
        0,       0,       1,       1,       1,       1,       1,       1,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        1,       0,       0,       0,       0,       0,       0,       0,
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
        0,       0,       0,       1,       1,       1,       1,       1,
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
        1,      'A',     'B',     'C',     'D',     'E',     'F',     'G',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
        'H',     'I',     'J',     'K',     'L',     'M',     'N',     'O',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
        'P',     'Q',     'R',     'S',     'T',     'U',     'V',     'W',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
        'X',     'Y',     'Z',      1,       1,      1,       1,       0 };

#define NT(t) transform_map[(unsigned char) t]

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

int pion_http_message_status(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    message->status = parser->status_code;
    message->reason = zend_string_init(at, length, 0);
    return 0;
}

int pion_http_header_name(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    message->parser.http->buffer = zend_string_init(at, length, 0);
    return 0;
}

int pion_http_header_value(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    zval             * header = NULL;
    zval               value;
    zend_string      * name_lower;
    if(!message->parser.http->buffer) {
        zend_error(E_NOTICE, "Unexpected HTTP parser behavior: header value without header name");
        return 1;
    }

    name_lower = zend_string_tolower(message->parser.http->buffer);

    header = zend_hash_find(message->headers, name_lower);
    if(!header) {
        zval h;
        array_init(&h);
        header = zend_hash_add(message->headers, name_lower, &h);
    }
    ZVAL_STR(&value, zend_string_init(at, length, 0));
    zend_hash_next_index_insert(Z_ARR_P(header), &value);

    zend_string_release(name_lower);
    zend_string_release(message->parser.http->buffer);
    message->parser.http->buffer = NULL;

    return 0;
}

int pion_http_message_body(http_parser * parser, const char * at, size_t length) {
    ion_http_message * message = parser->data;
    message->body = pion_http_buffering(message->body, at, length);
    return 0;
}


int pion_http_message_complete(http_parser * parser) {
    ion_http_message * message = parser->data;
    message->flags |= ION_HTTP_MESSAGE_COMPLETE;
    return 0;
}


zend_object * pion_http_parse_request(zend_string * request_string, zend_class_entry * ce) {
    zend_object          * request = pion_new_object_arg_0(ce ? ce : ion_ce_ION_HTTP_Request);
    ion_http_message     * message = get_object_instance(request, ion_http_message);
    http_parser_settings   settings;
    size_t                 nparsed;
    http_parser          * parser;


    message->parser.http = ecalloc(1, sizeof(ion_http_parser));
    parser = ecalloc(1, sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    message->parser.http->parser = parser;

    parser->data = message;
    memset(&settings, 0, sizeof(settings));
    settings.on_message_complete = pion_http_message_begin;
    settings.on_url              = pion_http_url;
    settings.on_header_field     = pion_http_header_name;
    settings.on_header_value     = pion_http_header_value;
    settings.on_message_complete = pion_http_message_complete;
    settings.on_body             = pion_http_message_body;

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
    efree(message->parser.http);
    return request;
}

zend_object * pion_http_parse_response(zend_string * response_string, zend_class_entry * ce) {
    zend_object          * response = pion_new_object_arg_0(ce ? ce : ion_ce_ION_HTTP_Response);
    ion_http_message     * message = get_object_instance(response, ion_http_message);
    http_parser_settings   settings;
    size_t                 nparsed;
    http_parser          * parser;


    message->parser.http = ecalloc(1, sizeof(ion_http_parser));
    parser = ecalloc(1, sizeof(http_parser));
    http_parser_init(parser, HTTP_RESPONSE);
    message->parser.http->parser = parser;

    parser->data = message;
    memset(&settings, 0, sizeof(settings));
    settings.on_message_complete = pion_http_message_begin;
    settings.on_header_field     = pion_http_header_name;
    settings.on_header_value     = pion_http_header_value;
    settings.on_message_complete = pion_http_message_complete;
    settings.on_body             = pion_http_message_body;
    settings.on_status           = pion_http_message_status;

    nparsed = http_parser_execute(parser, &settings, response_string->val, response_string->len);
    if(nparsed < response_string->len) {
        zend_error(E_NOTICE, "has unparsed data");
    }
    efree(parser);
    efree(message->parser.http);
    return response;
}

zend_string * pion_http_message_build(zend_object * message_object) {
    USE_ION_CACHE;

    ion_http_message * message = get_object_instance(message_object, ion_http_message);
    zend_string     ** pipe = emalloc(sizeof(zend_string *) * 32);
    size_t             pipe_size = 32;
    size_t             pipe_pos  = 0;
    size_t             message_size = 0;
    zend_string      * msg = NULL;
    zend_string      * buffer        = NULL;
    zend_string      * header = NULL;
    zend_ulong         header_id;
    zval             * value = NULL;
    zval             * v = NULL;


    // begin head
    if(message->type == ion_http_type_response) {
        pipe[0] = ION_STR_CACHE(ION_STR_UP_HTTP);  // 4 bytes
        pipe[1] = ION_STR_CACHE(ION_STR_SLASH);    // 1 byte
        pipe[2] = message->version ? message->version : ION_STR_CACHE(ION_STR_V11);
        pipe[3] = ION_STR_CACHE(ION_STR_SPACE);    // 1 byte
        pipe[4] = buffer = strpprintf(4, "%d", message->status);
        pipe[5] = ION_STR_CACHE(ION_STR_SPACE);    // 1 byte
        pipe[6] = message->reason ? message->reason : pion_http_reason(message->status);
        pipe[7] = ION_STR_CACHE(ION_STR_CRLF);     // 2 bytes
        message_size = pipe[2]->len + buffer->len + pipe[6]->len + 9;
        pipe_pos = 8;
    } else if(message->type == ion_http_type_request) {
        if(message->target) {
            buffer = zend_string_copy(message->target);
        } else if(message->uri) {
            buffer = ion_uri_stringify(message->uri, URI_ALL);
        }
        pipe[0] = message->method ? message->method : ION_STR_CACHE(ION_STR_UP_GET);
        pipe[1] = ION_STR_CACHE(ION_STR_SPACE);    // 1 byte
        pipe[2] = buffer ? buffer : ION_STR_CACHE(ION_STR_SLASH);
        pipe[3] = ION_STR_CACHE(ION_STR_SPACE);    // 1 byte
        pipe[4] = ION_STR_CACHE(ION_STR_UP_HTTP);  // 4 bytes
        pipe[5] = ION_STR_CACHE(ION_STR_SLASH);    // 1 byte
        pipe[6] = message->version ? message->version : ION_STR_CACHE(ION_STR_V11);
        pipe[7] = ION_STR_CACHE(ION_STR_CRLF);     // 2 bytes
        message_size = pipe[0]->len + pipe[2]->len + pipe[6]->len + 9;
        pipe_pos = 8;
    }
    // end head
    // begin headers
    ZEND_HASH_FOREACH_KEY_VAL(message->headers, header_id, header, value) {
        if(!header) {
            header = ION_STR_CACHE(header_id - ION_HTTP_HEADERS_STRINGS_OFFSET_TO_LOW);
        }
        if(zend_hash_num_elements(Z_ARR_P(value)) == 1) {
            if(pipe_pos + 4 > pipe_size) {
                pipe = erealloc(pipe, sizeof(zend_string *) * pipe_size * 2);
                pipe_size *= 2;
            }
            pipe[pipe_pos++] = header;
            pipe[pipe_pos++] = ION_STR_CACHE(ION_STR_COLON_SP);
            pipe[pipe_pos] = Z_STR_P(zend_hash_get_current_data(Z_ARR_P(value)));
            message_size += header->len + 4 + pipe[pipe_pos]->len;
            pipe_pos++;
            pipe[pipe_pos++] = ION_STR_CACHE(ION_STR_CRLF);
        } else {
            if(pipe_pos + 2 + zend_hash_num_elements(Z_ARR_P(value)) * 2 > pipe_size) {
                pipe = erealloc(pipe, sizeof(zend_string *) * pipe_size * 2);
                pipe_size *= 2;
            }
            pipe[pipe_pos++] = header;
            pipe[pipe_pos++] = ION_STR_CACHE(ION_STR_COLON_SP);
            message_size += header->len + 3;
            ZEND_HASH_FOREACH_VAL(Z_ARR_P(value), v) {
                pipe[pipe_pos++] = Z_STR_P(v);
                pipe[pipe_pos++] = ION_STR_CACHE(ION_STR_COMA);
                message_size += Z_STRLEN_P(v) + 1;
            } ZEND_HASH_FOREACH_END();
            pipe[pipe_pos-1] = ION_STR_CACHE(ION_STR_CRLF);
        }
    } ZEND_HASH_FOREACH_END();
    // end headers

    if(message->body) {
        message_size += message->body->len + 2;
    } else {
        message_size += 2;
    }

    msg = zend_string_alloc(message_size, 0);

    size_t offset = 0;
    zend_bool is_header = false;
    zend_bool to_up = false;
    for(size_t p = 0; p < pipe_pos; p++) {
        memcpy(&msg->val[offset], pipe[p]->val, pipe[p]->len);
        if(is_header) {
            for(size_t i = offset; i < offset + pipe[p]->len; i++) {
                if(NT(msg->val[i])) {
                    if(NT(msg->val[i]) == 1) {
                        to_up = true;
                    } else if(to_up) {
                        msg->val[i] = NT(msg->val[i]);
                        to_up = false;
                    }
                }
            }
            is_header = false;
        }
        offset += pipe[p]->len;
        if(pipe[p] == ION_STR_CACHE(ION_STR_CRLF)) {
            is_header = true;
            to_up = true;
        }
    }
    memcpy(&msg->val[offset], "\r\n", 2);
    if(message->body) {
        memcpy(&msg->val[offset + 2], message->body->val, message->body->len);
    }

    efree(pipe);
    if(buffer) {
        zend_string_release(buffer);
    }
    msg->val[msg->len] = '\000';

    return msg;
}

zend_string * pion_http_reason(uint16_t response_code) {
    switch(response_code) {
        case 100: return ION_STR(ION_STR_HTTP_100);
        case 101: return ION_STR(ION_STR_HTTP_101);
        case 102: return ION_STR(ION_STR_HTTP_102);
        case 200: return ION_STR(ION_STR_HTTP_200);
        case 201: return ION_STR(ION_STR_HTTP_201);
        case 202: return ION_STR(ION_STR_HTTP_202);
        case 203: return ION_STR(ION_STR_HTTP_203);
        case 204: return ION_STR(ION_STR_HTTP_204);
        case 205: return ION_STR(ION_STR_HTTP_205);
        case 206: return ION_STR(ION_STR_HTTP_206);
        case 207: return ION_STR(ION_STR_HTTP_207);
        case 226: return ION_STR(ION_STR_HTTP_226);
        case 300: return ION_STR(ION_STR_HTTP_300);
        case 301: return ION_STR(ION_STR_HTTP_301);
        case 302: return ION_STR(ION_STR_HTTP_302);
        case 303: return ION_STR(ION_STR_HTTP_303);
        case 304: return ION_STR(ION_STR_HTTP_304);
        case 305: return ION_STR(ION_STR_HTTP_305);
        case 307: return ION_STR(ION_STR_HTTP_307);
        case 400: return ION_STR(ION_STR_HTTP_400);
        case 401: return ION_STR(ION_STR_HTTP_401);
        case 402: return ION_STR(ION_STR_HTTP_402);
        case 403: return ION_STR(ION_STR_HTTP_403);
        case 404: return ION_STR(ION_STR_HTTP_404);
        case 405: return ION_STR(ION_STR_HTTP_405);
        case 406: return ION_STR(ION_STR_HTTP_406);
        case 407: return ION_STR(ION_STR_HTTP_407);
        case 408: return ION_STR(ION_STR_HTTP_408);
        case 409: return ION_STR(ION_STR_HTTP_409);
        case 410: return ION_STR(ION_STR_HTTP_410);
        case 411: return ION_STR(ION_STR_HTTP_411);
        case 412: return ION_STR(ION_STR_HTTP_412);
        case 413: return ION_STR(ION_STR_HTTP_413);
        case 414: return ION_STR(ION_STR_HTTP_414);
        case 415: return ION_STR(ION_STR_HTTP_415);
        case 416: return ION_STR(ION_STR_HTTP_416);
        case 417: return ION_STR(ION_STR_HTTP_417);
        case 418: return ION_STR(ION_STR_HTTP_418);
        case 422: return ION_STR(ION_STR_HTTP_422);
        case 423: return ION_STR(ION_STR_HTTP_423);
        case 424: return ION_STR(ION_STR_HTTP_424);
        case 425: return ION_STR(ION_STR_HTTP_425);
        case 426: return ION_STR(ION_STR_HTTP_426);
        case 428: return ION_STR(ION_STR_HTTP_428);
        case 429: return ION_STR(ION_STR_HTTP_429);
        case 431: return ION_STR(ION_STR_HTTP_431);
        case 434: return ION_STR(ION_STR_HTTP_434);
        case 449: return ION_STR(ION_STR_HTTP_449);
        case 451: return ION_STR(ION_STR_HTTP_451);
        case 500: return ION_STR(ION_STR_HTTP_500);
        case 501: return ION_STR(ION_STR_HTTP_501);
        case 502: return ION_STR(ION_STR_HTTP_502);
        case 503: return ION_STR(ION_STR_HTTP_503);
        case 504: return ION_STR(ION_STR_HTTP_504);
        case 505: return ION_STR(ION_STR_HTTP_505);
        case 506: return ION_STR(ION_STR_HTTP_506);
        case 507: return ION_STR(ION_STR_HTTP_507);
        case 508: return ION_STR(ION_STR_HTTP_508);
        case 509: return ION_STR(ION_STR_HTTP_509);
        case 510: return ION_STR(ION_STR_HTTP_510);
        case 511: return ION_STR(ION_STR_HTTP_511);
        default:
            return NULL;
    }
}