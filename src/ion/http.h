#ifndef PION_HTTP_H
#define PION_HTTP_H

#include "init.h"
#include "../deps/http-parser/http_parser.h"
#include "../deps/multipart-parser-c/multipart_parser.h"
#include "../deps/websocket-parser/websocket_parser.h"

extern ION_API zend_class_entry * ion_ce_ION_URI;

extern ION_API zend_class_entry * ion_ce_ION_HTTP_Message;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_Request;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_Response;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WSFrame;

extern ION_API zend_class_entry * ion_ce_ION_HTTP_ChunkEncoding;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_MultiParted;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_MultiParted_Part;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WebSocket;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WebSocket_Frame;

enum ion_http_msg_type {
    ion_http_type_message,
    ion_http_type_request,
    ion_http_type_response,
    ion_http_type_part,
};

enum ion_http_parser_type {
    ion_http_type_websocket,
    ion_http_type_multipart,
    ion_http_type_default,
};

#define ION_HTTP_VERSION_DEFAULT "1.1"
#define ION_HTTP_MESSAGE_REQUEST  1
#define ION_HTTP_MESSAGE_RESPONSE 2
#define ION_HTTP_MESSAGE_PART     4
#define ION_HTTP_MESSAGE_COMPLETE 8

#define ION_HTTP_METHOD_STR(method_num) ION_STR(method_num + ION_HTTP_METHOD_STRINGS_OFFSET)

#define URI_SCHEME    (1<<0)
#define URI_USER_NAME (1<<1)
#define URI_USER_PASS (1<<2)
#define URI_HOST      (1<<3)
#define URI_PORT      (1<<4)
#define URI_PATH      (1<<5)
#define URI_QUERY     (1<<6)
#define URI_FRAGMENT  (1<<7)

#define URI_ALL (URI_SCHEME | URI_USER_NAME | URI_USER_PASS | URI_HOST | URI_PORT | URI_PATH | URI_QUERY | URI_FRAGMENT)

typedef struct _ion_uri {
    zend_object   std;
    zend_string * scheme;
    zend_string * user;
    zend_string * pass;
    zend_string * host;
    unsigned short port;
    zend_string * path;
    zend_string * query;
    zend_string * fragment;
} ion_uri;

typedef struct _ion_http_parser {
    http_parser      * parser;
    multipart_parser * mp_parser;
    zend_string      * buffer;
} ion_http_parser;

typedef struct _ion_http_message {
    zend_object       std;
    uint8_t           type;
    uint8_t           flags;
    zend_array      * headers;
    zend_string     * version;
    zend_object     * stream;
    union {
        ion_http_parser  * http;
        multipart_parser * multipart;
        websocket_parser * websocket;
        zend_object      * custom;
    } parser;
    zend_string     * body;

    zend_object     * uri;
    uint16_t          status;
    zend_string     * method;
    zend_string     * reason;
    zend_string     * target;
} ion_http_message;

typedef struct _ion_http_request {
    zend_object       std;
    uint8_t           type;
    uint8_t           flags;
    zend_array      * headers;
    zend_string     * version;
    zend_object     * stream;
    ion_http_parser * parser;
    zend_string     * body;

    zend_object     * uri;
    zend_string     * method;
    zend_string     * target;
} ion_http_request;


typedef struct _ion_http_response {
    zend_object       std;
    uint8_t           type;
    uint8_t           flags;
    zend_array      * headers;
    zend_string     * version;
    zend_object     * stream;
    ion_http_parser * parser;
    zend_string     * body;

    uint32_t          code;
    zend_string     * reason;
} ion_http_response;

typedef struct _ion_http_websocket_frame {
    zend_object   std;
    websocket_flags flags;
    zend_string *   body;
    char            mask[4];
} ion_http_websocket_frame;

typedef struct _ion_http_multi_parted_parser {
    zend_object   std;
    multipart_parser * parser;
} ion_http_mp_parser;

typedef struct _ion_websocket_parser {
    websocket_parser p;
    zend_object * on_frame;
    ion_http_websocket_frame * frame;
    zend_ulong    frames_num;
} ion_websocket_parser;

typedef struct _ion_http_body_parser {
    zend_object   std;
    uint8_t       type;
    uint32_t      flags;
    union {
        http_parser          * http;
        multipart_parser     * multipart;
        ion_websocket_parser * websocket;
        zend_object          * custom;
    } parser;
    union {
        http_parser_settings      * http;
        multipart_parser_settings * multipart;
        websocket_parser_settings * websocket;
    } settings;
} ion_http_body_parser;

extern ION_API zend_string * ion_uri_stringify(zend_object * uri, unsigned short parts);
extern ION_API zend_object * ion_uri_parse(zend_string * uri_string);

extern ION_API zend_object * ion_http_message_init(zend_class_entry * ce);
extern ION_API void ion_http_message_free(zend_object * zo_message);
extern ION_API void ion_http_message_store_header_value(zend_array * header, zval * value);
extern ION_API void ion_http_message_with_header(zend_array * headers, zend_string * name, zval * value);
extern ION_API void ion_http_message_with_added_header(zend_array * headers, zend_string * name, zval * value);


extern ION_API zend_string * pion_http_reason(uint16_t response_code);

extern ION_API zend_object * pion_http_parse_request(zend_string * request_string, zend_class_entry * ce);
extern ION_API zend_string * pion_http_message_build(zend_object * message_object);
extern ION_API zend_object * pion_http_parse_response(zend_string * response_string, zend_class_entry * ce);
extern ION_API zend_object * pion_http_parse_ws_frame(zend_string * response_string);

extern ION_API zend_object * pion_http_mp_parser(zend_object * stream);
extern ION_API zend_object * pion_http_mp_part(zend_object * mp_parser);

#endif //PION_HTTP_H
