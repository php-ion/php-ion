#ifndef PION_SSL_H
#define PION_SSL_H

#include <openssl/ssl.h>
#include "init.h"

#define ION_CRYPTO_IS_CLIENT        (1<<0)
#define ION_CRYPTO_METHOD_SSLv2     (1<<1)
#define ION_CRYPTO_METHOD_SSLv3     (1<<2)
#define ION_CRYPTO_METHOD_TLSv10    (1<<3)
#define ION_CRYPTO_METHOD_TLSv11    (1<<4)
#define ION_CRYPTO_METHOD_TLSv12    (1<<5)
#define ION_CRYPTO_METHODS_MASK     (ION_CRYPTO_METHOD_SSLv2 | ION_CRYPTO_METHOD_SSLv3 \
    | ION_CRYPTO_METHOD_TLSv10 | ION_CRYPTO_METHOD_TLSv11 | ION_CRYPTO_METHOD_TLSv12)

#define ION_CRYPTO_ALLOW_SELF_SIGNED       (1<<10)
#define ION_CRYPTO_HAS_PASSPHRASE_CB       (1<<11)
#define ION_CRYPTO_PASSPHRASE_REQUESTED    (1<<12)

ION_API SSL * ion_crypto_server_stream_handler(ion_crypto * ssl);
ION_API SSL * ion_crypto_client_stream_handler(ion_crypto * ssl);

#define ion_crypto_check_is_client(obj) (get_object_instance(obj, ion_crypto)->flags & ION_CRYPTO_IS_CLIENT)

struct _ion_crypto {
//    zend_object   std;
    uint          flags;
    int           verify_depth;
    zend_string * passphrase;
    SSL_CTX     * ctx;

    zend_object   php_object;
};


extern ION_API zend_class_entry * ion_ce_ION_Crypto;
extern ION_API zend_class_entry * ion_ce_ION_CryptoException;

#endif //PION_SSL_H
