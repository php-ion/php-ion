#ifndef ION_CRYPTO_H
#define ION_CRYPTO_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "../pion.h"

#define ION_CRYPTO_CRYPTO_METHOD_AUTO       0
#define ION_CRYPTO_IS_CLIENT               (1<<0)
#define ION_CRYPTO_CRYPTO_METHOD_SSLv2     (1<<1)
#define ION_CRYPTO_CRYPTO_METHOD_SSLv3     (1<<2)
#define ION_CRYPTO_CRYPTO_METHOD_TLSv10    (1<<3)
#define ION_CRYPTO_CRYPTO_METHOD_TLSv11    (1<<4)
#define ION_CRYPTO_CRYPTO_METHOD_TLSv12    (1<<5)
#define ION_CRYPTO_CRYPTO_METHODS_MASK     (ION_CRYPTO_CRYPTO_METHOD_SSLv2 | ION_CRYPTO_CRYPTO_METHOD_SSLv3 \
    | ION_CRYPTO_CRYPTO_METHOD_TLSv10 | ION_CRYPTO_CRYPTO_METHOD_TLSv11 | ION_CRYPTO_CRYPTO_METHOD_TLSv12)

#define ION_CRYPTO_ALLOW_SELF_SIGNED       (1<<10)
#define ION_CRYPTO_HAS_PASSPHRASE_CB       (1<<11)
#define ION_CRYPTO_PASSPHRASE_REQUESTED    (1<<12)

typedef struct _ion_ssl {
    zend_object   std;
    uint          flags;
    int           verify_depth;
    zend_string * passphrase;
    SSL_CTX     * ctx;
} ion_ssl;

#endif //ION_CRYPTO_H
