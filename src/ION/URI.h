#ifndef ION_URI_H
#define ION_URI_H

extern ION_API zend_class_entry * ion_ce_ION_URI;

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

extern ION_API zend_string * ion_uri_stringify(zend_object * uri, unsigned short parts);
extern ION_API zend_object * ion_uri_parse(zend_string * uri_string);

#endif //ION_URI_H
