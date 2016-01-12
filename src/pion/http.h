#ifndef PION_HTTP_H
#define PION_HTTP_H

#include "init.h"
#include "../external/http-parser/http_parser.h"
#include "../external/multipart-parser-c/multipart_parser.h"

extern ION_API zend_class_entry * ion_ce_ION_URI;

extern ION_API zend_class_entry * ion_ce_ION_HTTP_Message;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_Request;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_Respose;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WSFrame;

extern ION_API zend_class_entry * ion_ce_ION_HTTP_ChunkEncoding;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_MultiParted;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_MultiParted_Part;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WebSocket;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WebSocket_Frame;

#define ION_HTTP_VERSION_DEFAULT "1.1"

#define ION_HTTP_MESSAGE_COMPLATE 1


#define URI_SCHEME   (1<<0)
#define URI_USER     (1<<1)
#define URI_PASS     (1<<2)
#define URI_HOST     (1<<3)
#define URI_PORT     (1<<4)
#define URI_PATH     (1<<5)
#define URI_QUERY    (1<<6)
#define URI_FRAGMENT (1<<7)

#define URI_ALL (URI_SCHEME | URI_USER | URI_PASS | URI_HOST | URI_PORT | URI_PATH | URI_QUERY | URI_FRAGMENT)

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


typedef struct _ion_http_message {
    zend_object   std;
    uint8_t       flags;
    zend_array  * headers;
    zend_object * uri;
    uint32_t      code;
    zend_string * reason;
    zend_string * version;
    zend_string * body;
    zend_object * stream;
    http_parser * parser;
} ion_http_message;

typedef struct _ion_http_multi_parted_parser {
    zend_object   std;
    multipart_parser * parser;
} ion_http_mp_parser;

extern ION_API zend_string * ion_uri_stringify(zend_object * uri, unsigned short parts);
extern ION_API zend_object * ion_uri_parse(zend_string * uri_string);

zend_object * ion_http_message_init(zend_class_entry * ce);
void ion_http_message_free(zend_object * zo_message);


extern ION_API const char * pion_http_reason(uint16_t response_code);

zend_object * pion_http_parse_request(zend_string * request_string, zend_class_entry * ce);
extern ION_API zend_object * pion_http_parse_respose(zend_string * response_string);
extern ION_API zend_object * pion_http_parse_ws_frame(zend_string * response_string);

extern ION_API zend_object * pion_http_mp_parser(zend_object * stream);
extern ION_API zend_object * pion_http_mp_part(zend_object * mp_parser);

#endif //PION_HTTP_H
