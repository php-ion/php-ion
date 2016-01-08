#ifndef ION_HTTP_MESSAGE_H
#define ION_HTTP_MESSAGE_H

extern ION_API zend_class_entry * ion_ce_ION_HTTP_Message;

typedef struct _ion_http_message {
    zend_object   std;
    uint8_t       flags;
    zend_array  * headers;
    zend_object * uri;
    uint32_t      code;
    zend_string * reason;
    zend_string * version;
} ion_http_message;

#endif //ION_HTTP_MESSAGE_H
