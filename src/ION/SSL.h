#ifndef ION_SSL_H
#define ION_SSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "../pion.h"

typedef struct _ion_ssl {
    zend_object   std;
    uint          flags;
    int           verify_depth;
    zend_string * passphrase;
    SSL_CTX     * ctx;
} ion_ssl;

#endif //ION_SSL_H
