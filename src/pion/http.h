#ifndef PION_HTTP_H
#define PION_HTTP_H

#include "init.h"
#include "../external/http-parser/http_parser.h"
#include "../external/multipart-parser-c/multipart_parser.h"

extern ION_API zend_class_entry * ion_ce_ION_HTTP_Message;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_Request;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_Respose;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WSFrame;

extern ION_API zend_class_entry * ion_ce_ION_HTTP_ChunkEncoding;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_MultiParted;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_MultiParted_Part;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WebSocket;
extern ION_API zend_class_entry * ion_ce_ION_HTTP_WebSocket_Frame;

#define ION_HTTP_MSG_REQUEST 1
#define ION_HTTP_MSG_RESPOSE 2

typedef struct _ion_http_message {
    zend_object   std;
    uint8_t       flags;
    zend_array  * headers;
    zend_object * uri;
    uint32_t      code;
    zend_string * reason;
    zend_string * version;
    zend_object * stream;
    http_parser * parser;
} ion_http_message;

typedef struct _ion_http_multi_parted_parser {
    zend_object   std;
    multipart_parser * parser;
} ion_http_mp_parser;


extern ION_API zend_object * pion_http_parse_request(zend_string * request_string);
extern ION_API zend_object * pion_http_parse_respose(zend_string * response_string);
extern ION_API zend_object * pion_http_parse_ws_frame(zend_string * response_string);

extern ION_API zend_object * pion_http_mp_parser(zend_object * stream);
extern ION_API zend_object * pion_http_mp_part(zend_object * mp_parser);

#endif //PION_HTTP_H
