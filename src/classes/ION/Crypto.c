#include "ion.h"
#include <openssl/rand.h>
#include <openssl/conf.h>

zend_class_entry     * ion_ce_ION_Crypto;
zend_object_handlers   ion_oh_ION_Crypto;
zend_class_entry     * ion_ce_ION_CryptoException;
zend_object_handlers   ion_oh_ION_CryptoException;

#define ION_CRYPTO_DEFAULT_CIPHERS "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:"

/* OpenSSL 1.0.2 removes SSLv2 support entirely*/
#if OPENSSL_VERSION_NUMBER < 0x10002000L && !defined(OPENSSL_NO_SSL2)
#define HAVE_SSL2 1
#endif

#ifndef OPENSSL_NO_SSL3
#define HAVE_SSL3 1
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10001001L
#define HAVE_TLS11 1
#define HAVE_TLS12 1
#endif

#if !defined(OPENSSL_NO_ECDH) && OPENSSL_VERSION_NUMBER >= 0x0090800fL
#define HAVE_ECDH 1
#endif

#if !defined(OPENSSL_NO_TLSEXT)
#if OPENSSL_VERSION_NUMBER >= 0x00908070L
#define HAVE_TLS_SNI 1
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
#define HAVE_TLS_ALPN 1
#endif
#endif

ION_API SSL * ion_crypto_server_stream_handler(ion_crypto * issl) {
    if(!(issl->flags & ION_CRYPTO_IS_CLIENT)) {
        return SSL_new(issl->ctx);
    }
    return NULL;
}

ION_API SSL * ion_crypto_client_stream_handler(ion_crypto * issl) {
    if(issl->flags & ION_CRYPTO_IS_CLIENT) {
        return SSL_new(issl->ctx);
    }
    return NULL;
}

static const SSL_METHOD * ion_crypto_select_crypto_method(zend_long method_value, zend_bool is_client)
{
    if (method_value == ION_CRYPTO_METHOD_SSLv2) {
#ifdef HAVE_SSL2
        return is_client ? (SSL_METHOD *)SSLv2_client_method() : (SSL_METHOD *)SSLv2_server_method();
#else
        zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_SSL2_DISABLED, 0);
        return NULL;
#endif
    } else if (method_value == ION_CRYPTO_METHOD_SSLv3) {
#ifdef HAVE_SSL3
        return is_client ? SSLv3_client_method() : SSLv3_server_method();
#else
        zend_throw_exception(ion_ce_ION_CryptoException, "SSLv3 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else if (method_value == ION_CRYPTO_METHOD_TLSv10) {
        return is_client ? TLSv1_client_method() : TLSv1_server_method();
    } else if (method_value == ION_CRYPTO_METHOD_TLSv11) {
#ifdef HAVE_TLS11
        return is_client ? TLSv1_1_client_method() : TLSv1_1_server_method();
#else
        zend_throw_exception(ion_ce_ION_CryptoException, "TLSv1.1 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else if (method_value == ION_CRYPTO_METHOD_TLSv12) {
#ifdef HAVE_TLS12
        return is_client ? TLSv1_2_client_method() : TLSv1_2_server_method();
#else
        zend_throw_exception(ion_ce_ION_CryptoException, "TLSv1.2 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else {
        zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_INVALID_METHOD, 0);
        return NULL;
    }
}


zend_long ion_crypto_get_crypto_method_ctx_flags(zend_long method_flags) {
    zend_long ssl_ctx_options = SSL_OP_ALL;

#ifdef HAVE_SSL2
    if (!(method_flags & ION_CRYPTO_METHOD_SSLv2)) {
		ssl_ctx_options |= SSL_OP_NO_SSLv2;
	}
#endif
#ifdef HAVE_SSL3
    if (!(method_flags & ION_CRYPTO_METHOD_SSLv3)) {
		ssl_ctx_options |= SSL_OP_NO_SSLv3;
	}
#endif
    if (!(method_flags & ION_CRYPTO_METHOD_TLSv10)) {
        ssl_ctx_options |= SSL_OP_NO_TLSv1;
    }
#ifdef HAVE_TLS11
    if (!(method_flags & ION_CRYPTO_METHOD_TLSv11)) {
		ssl_ctx_options |= SSL_OP_NO_TLSv1_1;
	}
#endif
#ifdef HAVE_TLS12
    if (!(method_flags & ION_CRYPTO_METHOD_TLSv12)) {
		ssl_ctx_options |= SSL_OP_NO_TLSv1_2;
	}
#endif

    return ssl_ctx_options;
}

static int ion_crypto_verify_cb(int preverify_ok, X509_STORE_CTX * ctx) {
    SSL     * ssl;
    int       ret = preverify_ok;
    int       err = 0;
    int       depth;
    ion_crypto * issl;

    ssl  = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    issl = (ion_crypto *) SSL_get_ex_data(ssl, GION(ssl_index));


    X509_STORE_CTX_get_current_cert(ctx);
    err      = X509_STORE_CTX_get_error(ctx);
    depth    = X509_STORE_CTX_get_error_depth(ctx);

    /* if allow_self_signed is set, make sure that verification succeeds */
    if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT && (issl->flags & ION_CRYPTO_ALLOW_SELF_SIGNED)) {
        ret = 1;
    }

    /* check the verify depth */
    if ((zend_ulong)depth > issl->verify_depth) {
        ret = 0;
        X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
    }

    return ret;
}

static int ion_crypto_passwd_cb(char * buf, int num, int verify, void * data) {
    ion_crypto * ssl = (ion_crypto *)data;

    if (ssl->passphrase) {
        if (ZSTR_LEN(ssl->passphrase) < num - 1) {
            memcpy(buf, ZSTR_VAL(ssl->passphrase), ZSTR_LEN(ssl->passphrase) + 1);
            return (int)ZSTR_LEN(ssl->passphrase);
        }
    } else {
        ssl->flags |= ION_CRYPTO_PASSPHRASE_REQUESTED;
    }
    return 0;
}

zend_object * ion_crypto_init(zend_class_entry * ce) {
    ion_crypto * ssl = ion_alloc_object(ce, ion_crypto);
    ssl->verify_depth = -1;
    return ion_init_object(ION_OBJECT_ZOBJ(ssl), ce, &ion_oh_ION_Crypto);
}

void ion_crypto_free(zend_object * object) {
    ion_crypto * ssl = ION_ZOBJ_OBJECT(object, ion_crypto);
    zend_object_std_dtor(object);
    if(ssl->ctx) {
        SSL_CTX_free(ssl->ctx);
        ssl->ctx = NULL;
    }
    if(ssl->passphrase) {
        zend_string_release(ssl->passphrase);
        ssl->passphrase = NULL;
    }

}

zend_object * ion_crypto_factory(zend_long flags) {
    zval        zssl;
    ion_crypto * ssl;
    zend_long   ssl_ctx_options;
    zend_bool   is_client = (flags & ION_CRYPTO_IS_CLIENT) ? true : false;
    zend_long   crypt_method = flags & ION_CRYPTO_METHODS_MASK;
    const SSL_METHOD * method;

    object_init_ex(&zssl, ion_ce_ION_Crypto);
    ssl = ION_ZVAL_OBJECT(zssl, ion_crypto);
    ssl->flags |= flags;

    if (has_one_bit(crypt_method)) { // use a specific crypto method
        ssl_ctx_options = SSL_OP_ALL;
        method = ion_crypto_select_crypto_method(crypt_method, is_client);
        if (method == NULL) {
            if(!EG(exception)) {
                zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_INVALID_METHOD, 0);
            }
            return NULL;
        }
    } else if(crypt_method) { // use generic SSLv23
        method = is_client ? SSLv23_client_method() : SSLv23_server_method();
        ssl_ctx_options = SSL_OP_ALL;
        ssl_ctx_options = ion_crypto_get_crypto_method_ctx_flags(crypt_method);
        if (ssl_ctx_options == -1) {
            zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_INVALID_METHOD, 0);
            return NULL;
        }
    } else {
        zend_throw_exception(ion_ce_InvalidArgumentException, ERR_ION_CRYPTO_NO_METHOD, 0);
        return NULL;
    }
#if OPENSSL_VERSION_NUMBER >= 0x10001001L
    ssl->ctx = SSL_CTX_new(method);
#else
    /* Avoid const warning with old versions */
	ssl->ctx = SSL_CTX_new((SSL_METHOD*)method);
#endif

    if (ssl->ctx == NULL) {
        zval_ptr_dtor(&zssl);
        zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_INIT_FAILED, 0);
        return NULL;
    }

//    if (SSL_CTX_set_cipher_list(ssl->ctx, ION_CRYPTO_DEFAULT_CIPHERS) != 1) {
//        zend_throw_exception_ex(ion_ce_ION_CryptoException, 0, "Failed setting cipher list: %s", ION_CRYPTO_DEFAULT_CIPHERS);
//        return NULL;
//    }

#if OPENSSL_VERSION_NUMBER >= 0x0090605fL
    ssl_ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif

    SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_PEER, ion_crypto_verify_cb);

    if ((ssl->flags & ION_CRYPTO_IS_CLIENT) && !SSL_CTX_set_default_verify_paths(ssl->ctx)) {
        zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_DEF_VERIFY_FAILED, 0);
        return NULL;
    }

    SSL_CTX_set_options(ssl->ctx, ssl_ctx_options);
    SSL_CTX_set_session_id_context(ssl->ctx, (unsigned char *)(void *)ssl->ctx, sizeof(ssl->ctx));

    return Z_OBJ(zssl);
}

CLASS_METHOD(ION_Crypto, server) {
    zend_object * object;
    zend_long     crypt_method = ION_CRYPTO_METHODS_MASK;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(crypt_method)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    object = ion_crypto_factory(crypt_method);
    if(object) {
        RETURN_OBJ(object);
    }
}

METHOD_ARGS_BEGIN(ION_Crypto, server, 0)
    ARGUMENT(method, IS_LONG)
METHOD_ARGS_END()

CLASS_METHOD(ION_Crypto, client) {
    zend_object * object;
    zend_long     crypt_method = ION_CRYPTO_METHODS_MASK;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(crypt_method)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    object = ion_crypto_factory(crypt_method | ION_CRYPTO_IS_CLIENT);
    if(object) {
        RETURN_OBJ(object);
    }
}

METHOD_ARGS_BEGIN(ION_Crypto, client, 0)
                ARGUMENT(method, IS_LONG)
METHOD_ARGS_END()

CLASS_METHOD(ION_Crypto, __construct) {}

METHOD_WITHOUT_ARGS(ION_Crypto, __construct)

CLASS_METHOD(ION_Crypto, ticket) {
    zend_bool   status = 0;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(status)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

#if OPENSSL_VERSION_NUMBER >= 0x0090806fL
    if(status) {
        SSL_CTX_clear_options(ssl->ctx, SSL_OP_NO_TICKET);
    } else {
        SSL_CTX_set_options(ssl->ctx, SSL_OP_NO_TICKET);
    }
#endif

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, ticket, 0)
    ARGUMENT(status, IS_BOOLEAN)
METHOD_ARGS_END()

CLASS_METHOD(ION_Crypto, compression) {
    zend_bool   status = 0;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(status)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
    if(status) {
        SSL_CTX_clear_options(ssl->ctx, SSL_OP_NO_COMPRESSION);
    } else {
        SSL_CTX_set_options(ssl->ctx, SSL_OP_NO_COMPRESSION);
    }
#endif

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, compression, 0)
    ARGUMENT(status, IS_BOOLEAN)
METHOD_ARGS_END()

/* public function ION\SSL::verifyPeer(bool $status = true) : self */
CLASS_METHOD(ION_Crypto, verifyPeer) {
    zend_bool   status = 0;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(status)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(status) {
        SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_PEER, ion_crypto_verify_cb);
    } else {
        SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_NONE, NULL);
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, verifyPeer, 0)
    ARGUMENT(status, IS_BOOLEAN)
METHOD_ARGS_END()

/* public function ION\SSL::verifyDepth(int $depth = -1) : self */
CLASS_METHOD(ION_Crypto, verifyDepth) {
    zend_long   depth = -1;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(depth)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ssl->verify_depth = (int)depth;

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, verifyDepth, 0)
    ARGUMENT(depth, IS_BOOLEAN)
METHOD_ARGS_END()

/* public function ION\SSL::localCert(string $local_cert, string $local_pk = null) : self */
CLASS_METHOD(ION_Crypto, localCert) {
    zend_string * local_cert = NULL;
    zend_string * local_pk = NULL;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_STR(local_cert)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_EX(local_pk, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    char resolved_path_buff[MAXPATHLEN];

    if (VCWD_REALPATH(local_cert->val, resolved_path_buff)) {
        if(!(ssl->flags & ION_CRYPTO_HAS_PASSPHRASE_CB)) {
            SSL_CTX_set_default_passwd_cb_userdata(ssl->ctx, ssl);
            SSL_CTX_set_default_passwd_cb(ssl->ctx, ion_crypto_passwd_cb);
            ssl->flags |= ION_CRYPTO_HAS_PASSPHRASE_CB;
        }
        /* a certificate to use for authentication */
        if (SSL_CTX_use_certificate_chain_file(ssl->ctx, resolved_path_buff) != 1) {
            zend_throw_exception_ex(ion_ce_ION_CryptoException, 0, ERR_ION_CRYPTO_CERT_CHAIN_FAILED, local_cert->val);
            return;
        }
        if (local_pk) {
            char resolved_path_buff_pk[MAXPATHLEN];
            if (VCWD_REALPATH(local_pk->val, resolved_path_buff_pk)) {
                if (SSL_CTX_use_PrivateKey_file(ssl->ctx, resolved_path_buff_pk, SSL_FILETYPE_PEM) != 1) {
                    zend_throw_exception_ex(ion_ce_ION_CryptoException,0, ERR_ION_CRYPTO_PKEY_FAILED, resolved_path_buff_pk,
                        (ssl->flags & ION_CRYPTO_PASSPHRASE_REQUESTED) ? ERR_ION_CRYPTO_PKEY_FAILED_PASSPHRASE : ""
                    );
                    return;
                }
            }
        } else {
            if (SSL_CTX_use_PrivateKey_file(ssl->ctx, resolved_path_buff, SSL_FILETYPE_PEM) != 1) {
                zend_throw_exception_ex(ion_ce_ION_CryptoException, 0, ERR_ION_CRYPTO_PKEY_FAILED, resolved_path_buff,
                    (ssl->flags & ION_CRYPTO_PASSPHRASE_REQUESTED) ? ERR_ION_CRYPTO_PKEY_FAILED_PASSPHRASE: ""
                );
                return;
            }
        }

#if OPENSSL_VERSION_NUMBER < 0x10001001L
        do {
				/* Unnecessary as of OpenSSLv1.0.1 (will segfault if used with >= 10001001 ) */
				X509 *cert = NULL;
				EVP_PKEY *key = NULL;
				SSL *tmpssl = SSL_new(ssl->ctx);
				cert = SSL_get_certificate(tmpssl);

				if (cert) {
					key = X509_get_pubkey(cert);
					EVP_PKEY_copy_parameters(key, SSL_get_privatekey(tmpssl));
					EVP_PKEY_free(key);
				}
				SSL_free(tmpssl);
			} while (0);
#endif
        if (!SSL_CTX_check_private_key(ssl->ctx)) {
            zend_throw_exception(ion_ce_ION_CryptoException, ERR_ION_CRYPTO_PKEY_MISSED, 0);
            return;
        }
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, localCert, 1)
    ARGUMENT(local_cert, IS_STRING)
    ARGUMENT(local_pk, IS_STRING)
METHOD_ARGS_END()

/* public function ION\SSL::passPhrase(string $phrase) : self */
CLASS_METHOD(ION_Crypto, passPhrase) {
    zend_string * phrase = NULL;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(phrase)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(!(ssl->flags & ION_CRYPTO_HAS_PASSPHRASE_CB)) {
        SSL_CTX_set_default_passwd_cb_userdata(ssl->ctx, ssl);
        SSL_CTX_set_default_passwd_cb(ssl->ctx, ion_crypto_passwd_cb);
        ssl->flags |= ION_CRYPTO_HAS_PASSPHRASE_CB;
    }
    if(ssl->passphrase) {
        zend_string_release(ssl->passphrase);
    }
    ssl->passphrase = zend_string_copy(phrase);
    ssl->flags &= ~ION_CRYPTO_PASSPHRASE_REQUESTED;

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, passPhrase, 1)
                ARGUMENT(phrase, IS_STRING)
METHOD_ARGS_END()

/* public function ION\SSL::allowSelfSigned(bool $state = true) : self */
CLASS_METHOD(ION_Crypto, allowSelfSigned) {
    zend_bool   state = true;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(state)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(state) {
        ssl->flags |= ION_CRYPTO_ALLOW_SELF_SIGNED;
    } else {
        ssl->flags &= ~ION_CRYPTO_ALLOW_SELF_SIGNED;
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, allowSelfSigned, 0)
    ARGUMENT(state, IS_BOOLEAN)
METHOD_ARGS_END()

/* public function ION\SSL::ca(string $cafile, string $capath) : self */
CLASS_METHOD(ION_Crypto, ca) {
    zend_string * cafile = NULL;
    zend_string * capath = NULL;
    char * cafile_str = NULL;
    char * capath_str = NULL;
    ion_crypto * ssl = ION_THIS_OBJECT(ion_crypto);

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_EX(cafile, 1, 0)
        Z_PARAM_STR_EX(capath, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(cafile) {
        cafile_str = cafile->val;
        if(!(ssl->flags & ION_CRYPTO_IS_CLIENT)) {
            /* Servers need to load and assign CA names from the cafile */
            STACK_OF(X509_NAME) *cert_names = SSL_load_client_CA_file(cafile->val);
            if (cert_names != NULL) {
                SSL_CTX_set_client_CA_list(ssl->ctx, cert_names);
            } else {
                zend_throw_exception_ex(ion_ce_ION_CryptoException, 0, ERR_ION_CRYPTO_CAFILE_FAILED, cafile->val);
                return;
            }
        }
    }

    if(capath) {
        capath_str = capath->val;
    }

    if (cafile || capath) {
        if (!SSL_CTX_load_verify_locations(ssl->ctx, cafile_str, capath_str)) {
            zend_throw_exception_ex(ion_ce_ION_CryptoException, 0, ERR_ION_CRYPTO_LOAD_VERIFY_FAILED, cafile_str, capath_str);
            return;
        }
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Crypto, ca, 1)
    ARGUMENT(cafile, IS_STRING)
    ARGUMENT(capath, IS_STRING)
METHOD_ARGS_END()


METHODS_START(methods_ION_Crypto)
    METHOD(ION_Crypto, server,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Crypto, client,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Crypto, __construct,     ZEND_ACC_PRIVATE)
    METHOD(ION_Crypto, ticket,          ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, compression,     ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, verifyPeer,      ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, verifyDepth,     ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, localCert,       ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, passPhrase,      ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, allowSelfSigned, ZEND_ACC_PUBLIC)
    METHOD(ION_Crypto, ca,              ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_Crypto) {
    zend_long  methods = 0;

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    OpenSSL_add_all_algorithms();
    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll()) {
        zend_error(E_ERROR, ERR_ION_CRYPTO_ENTROPY_FAILED);
        return FAILURE;
    }

    GION(ssl_index) = SSL_get_ex_new_index(0, "PHP ION index", NULL, NULL, NULL);

    if (false) {
//        OPENSSL_config();
    }

    ion_register_class(ion_ce_ION_Crypto, "ION\\Crypto", ion_crypto_init, methods_ION_Crypto);
    ion_init_object_handlers(ion_oh_ION_Crypto);
    ion_oh_ION_Crypto.free_obj = ion_crypto_free;
    ion_oh_ION_Crypto.clone_obj = NULL;
    ion_oh_ION_Crypto.offset = ion_offset(ion_crypto);

#ifdef HAVE_SSL2
    methods |= ION_CRYPTO_METHOD_SSLv2;
#endif
#ifdef HAVE_SSL3
    methods |= ION_CRYPTO_METHOD_SSLv3;
#endif
    methods |= ION_CRYPTO_METHOD_TLSv10;
#ifdef HAVE_TLS11
    methods |= ION_CRYPTO_METHOD_TLSv11;
#endif
#ifdef HAVE_TLS12
    methods |= ION_CRYPTO_METHOD_TLSv12;
#endif

    ion_class_declare_constant_long(ion_ce_ION_Crypto, "SUPPORTED_METHODS",  methods);

    ion_class_declare_constant_long(ion_ce_ION_Crypto, "METHOD_SSLv2",  ION_CRYPTO_METHOD_SSLv2);
    ion_class_declare_constant_long(ion_ce_ION_Crypto, "METHOD_SSLv3",  ION_CRYPTO_METHOD_SSLv3);
    ion_class_declare_constant_long(ion_ce_ION_Crypto, "METHOD_TLSv10",   ION_CRYPTO_METHOD_TLSv10);
    ion_class_declare_constant_long(ion_ce_ION_Crypto, "METHOD_TLSv11",   ION_CRYPTO_METHOD_TLSv11);
    ion_class_declare_constant_long(ion_ce_ION_Crypto, "METHOD_TLSv12",   ION_CRYPTO_METHOD_TLSv12);

    ion_register_exception(ion_ce_ION_CryptoException, ion_ce_ION_RuntimeException, "ION\\CryptoException");

    return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(ION_Crypto) {
    EVP_cleanup();

    return SUCCESS;
}