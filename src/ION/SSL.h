#ifndef ION_SSL_H
#define ION_SSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "../pion.h"

#define ION_SSL_CRYPTO_METHOD_AUTO       0
#define ION_SSL_IS_CLIENT               (1<<0)
#define ION_SSL_CRYPTO_METHOD_SSLv2     (1<<1)
#define ION_SSL_CRYPTO_METHOD_SSLv3     (1<<2)
#define ION_SSL_CRYPTO_METHOD_TLSv10    (1<<3)
#define ION_SSL_CRYPTO_METHOD_TLSv11    (1<<4)
#define ION_SSL_CRYPTO_METHOD_TLSv12    (1<<5)
#define ION_SSL_CRYPTO_METHODS_MASK     (ION_SSL_CRYPTO_METHOD_SSLv2 | ION_SSL_CRYPTO_METHOD_SSLv3 \
    | ION_SSL_CRYPTO_METHOD_TLSv10 | ION_SSL_CRYPTO_METHOD_TLSv11 | ION_SSL_CRYPTO_METHOD_TLSv12)

#define ION_SSL_ALLOW_SELF_SIGNED       (1<<10)
#define ION_SSL_HAS_PASSPHRASE_CB       (1<<11)
#define ION_SSL_PASSPHRASE_REQUESTED    (1<<12)

typedef struct _ion_ssl {
    zend_object   std;
    uint          flags;
    int           verify_depth;
    zend_string * passphrase;
    SSL_CTX     * ctx;
} ion_ssl;

#endif //ION_SSL_H
