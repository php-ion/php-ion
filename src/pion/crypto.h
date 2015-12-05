#ifndef PION_SSL_H
#define PION_SSL_H

#include <openssl/ssl.h>
#include "init.h"

ION_API SSL * ion_crypto_server_stream_handler(zend_object * ssl);
ION_API SSL * ion_crypto_client_stream_handler(zend_object * ssl);

extern ION_API zend_class_entry * ion_ce_ION_Crypto;
extern ION_API zend_class_entry * ion_ce_ION_CryptoException;

#endif //PION_SSL_H
