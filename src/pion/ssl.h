#ifndef ION_SSL_H
#define ION_SSL_H

#include <openssl/ssl.h>
#include "init.h"

ION_API SSL * ion_ssl_server_stream_handler(zend_object * ssl);
ION_API SSL * ion_ssl_client_stream_handler(zend_object * ssl);

#endif //ION_SSL_H
