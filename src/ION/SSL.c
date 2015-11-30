
#include "SSL.h"

zend_class_entry     * ion_ce_ION_SSL;
zend_object_handlers   ion_oh_ION_SSL;
zend_class_entry     * ion_ce_ION_SSLException;
zend_object_handlers   ion_oh_ION_SSLException;

#define ION_SSL_DEFAULT_CIPHERS "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:"

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

ION_API SSL * ion_ssl_server_stream_handler(zend_object * ssl) {
    ion_ssl * issl = get_object_instance(ssl, ion_ssl);
    if(!(issl->flags & ION_SSL_IS_CLIENT)) {
        return SSL_new(issl->ctx);
    }
    return NULL;
}

ION_API SSL * ion_ssl_client_stream_handler(zend_object * ssl) {
    ion_ssl * issl = get_object_instance(ssl, ion_ssl);
    if(issl->flags & ION_SSL_IS_CLIENT) {
        return SSL_new(issl->ctx);
    }
    return NULL;
}

static const SSL_METHOD * ion_ssl_select_crypto_method(zend_long method_value, zend_bool is_client)
{
    if (method_value == ION_SSL_CRYPTO_METHOD_SSLv2) {
#ifdef HAVE_SSL2
        return is_client ? (SSL_METHOD *)SSLv2_client_method() : (SSL_METHOD *)SSLv2_server_method();
#else
        zend_throw_exception(ion_ce_ION_SSLException, "SSLv2 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else if (method_value == ION_SSL_CRYPTO_METHOD_SSLv3) {
#ifdef HAVE_SSL3
        return is_client ? SSLv3_client_method() : SSLv3_server_method();
#else
        zend_throw_exception(ion_ce_ION_SSLException, "SSLv3 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else if (method_value == ION_SSL_CRYPTO_METHOD_TLSv10) {
        return is_client ? TLSv1_client_method() : TLSv1_server_method();
    } else if (method_value == ION_SSL_CRYPTO_METHOD_TLSv11) {
#ifdef HAVE_TLS11
        return is_client ? TLSv1_1_client_method() : TLSv1_1_server_method();
#else
        zend_throw_exception(ion_ce_ION_SSLException, "TLSv1.1 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else if (method_value == ION_SSL_CRYPTO_METHOD_TLSv12) {
#ifdef HAVE_TLS12
        return is_client ? TLSv1_2_client_method() : TLSv1_2_server_method();
#else
        zend_throw_exception(ion_ce_ION_SSLException, "TLSv1.2 unavailable in the OpenSSL library against which PHP is linked", 0);
        return NULL;
#endif
    } else {
        zend_throw_exception(ion_ce_ION_SSLException, "Invalid crypto method", 0);
        return NULL;
    }
}


zend_long ion_ssl_get_crypto_method_ctx_flags(zend_long method_flags) {
    zend_long ssl_ctx_options = SSL_OP_ALL;

#ifdef HAVE_SSL2
    if (!(method_flags & ION_SSL_CRYPTO_METHOD_SSLv2)) {
		ssl_ctx_options |= SSL_OP_NO_SSLv2;
	}
#endif
#ifdef HAVE_SSL3
    if (!(method_flags & ION_SSL_CRYPTO_METHOD_SSLv3)) {
		ssl_ctx_options |= SSL_OP_NO_SSLv3;
	}
#endif
    if (!(method_flags & ION_SSL_CRYPTO_METHOD_TLSv10)) {
        ssl_ctx_options |= SSL_OP_NO_TLSv1;
    }
#ifdef HAVE_TLS11
    if (!(method_flags & ION_SSL_CRYPTO_METHOD_TLSv11)) {
		ssl_ctx_options |= SSL_OP_NO_TLSv1_1;
	}
#endif
#ifdef HAVE_TLS12
    if (!(method_flags & ION_SSL_CRYPTO_METHOD_TLSv12)) {
		ssl_ctx_options |= SSL_OP_NO_TLSv1_2;
	}
#endif

    return ssl_ctx_options;
}

static int ion_ssl_verify_cb(int preverify_ok, X509_STORE_CTX * ctx) {
    SSL     * ssl;
    int       ret = 0;
    int       err;
    int       depth;
    ion_ssl * issl;

    ssl  = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    issl = (ion_ssl *) SSL_get_ex_data(ssl, GION(ssl_index));


    X509_STORE_CTX_get_current_cert(ctx);
    err      = X509_STORE_CTX_get_error(ctx);
    depth    = X509_STORE_CTX_get_error_depth(ctx);

    /* if allow_self_signed is set, make sure that verification succeeds */
    if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT && (issl->flags & ION_SSL_ALLOW_SELF_SIGNED)) {
        ret = 1;
    }

    /* check the verify depth */
    if ((zend_ulong)depth > issl->verify_depth) {
        ret = 0;
        X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
    }

    return ret;
}

static int ion_ssl_passwd_cb(char * buf, int num, int verify, void * data) {
    ion_ssl * ssl = (ion_ssl *)data;

    if (ssl->passphrase) {
        if (ZSTR_LEN(ssl->passphrase) < num - 1) {
            memcpy(buf, ZSTR_VAL(ssl->passphrase), ZSTR_LEN(ssl->passphrase) + 1);
            return (int)ZSTR_LEN(ssl->passphrase);
        }
    }
    return 0;
}

zend_object * ion_ssl_init(zend_class_entry * ce) {
    ion_ssl * ssl = ecalloc(1, sizeof(ion_stream));
    ssl->verify_depth = -1;
    RETURN_INSTANCE(ION_SSL, ssl);
}

void ion_ssl_free(zend_object * object) {
    ion_ssl * ssl = get_object_instance(object, ion_ssl);
    if(ssl->ctx) {
        SSL_CTX_free(ssl->ctx);
        ssl->ctx = NULL;
    }
    if(ssl->passphrase) {
        zend_string_release(ssl->passphrase);
        ssl->passphrase = NULL;
    }

}

zend_object * ion_ssl_factory(zend_long flags) {
    zval        zssl;
    ion_ssl   * ssl;
    zend_long   ssl_ctx_options;
    zend_bool   is_client = (flags & ION_SSL_IS_CLIENT) ? true : false;
    zend_long   crypt_method = flags & ION_SSL_CRYPTO_METHODS_MASK;
    const SSL_METHOD * method;

    object_init_ex(&zssl, ion_ce_ION_SSL);
    ssl = get_instance(&zssl, ion_ssl);
    ssl->flags |= flags;

    if (crypt_method) { // use a specific crypto method
        ssl_ctx_options = SSL_OP_ALL;
        method = ion_ssl_select_crypto_method(crypt_method, is_client);
        if (method == NULL) {
            if(!EG(exception)) {
                zend_throw_exception(ion_ce_ION_SSLException, "Invalid crypt method", 0);
            }
            return NULL;
        }
    } else { // use generic SSLv23
        method = is_client ? SSLv23_client_method() : SSLv23_server_method();
        ssl_ctx_options = SSL_OP_ALL;
//        ssl_ctx_options = ion_ssl_get_crypto_method_ctx_flags(crypt_method);
        if (ssl_ctx_options == -1) {
            zend_throw_exception(ion_ce_ION_SSLException, "Invalid crypt method", 0);
            return NULL;
        }
    }
#if OPENSSL_VERSION_NUMBER >= 0x10001001L
    ssl->ctx = SSL_CTX_new(method);
#else
    /* Avoid const warning with old versions */
	ssl->ctx = SSL_CTX_new((SSL_METHOD*)method);
#endif

    if (ssl->ctx == NULL) {
        zval_ptr_dtor(&zssl);
        zend_throw_exception(ion_ce_ION_SSLException, "SSL context creation failure", 0);
        return NULL;
    }

    if (SSL_CTX_set_cipher_list(ssl->ctx, ION_SSL_DEFAULT_CIPHERS) != 1) {
        zend_throw_exception_ex(ion_ce_ION_SSLException, 0, "Failed setting cipher list: %s", ION_SSL_DEFAULT_CIPHERS);
        return NULL;
    }

#if OPENSSL_VERSION_NUMBER >= 0x0090605fL
    ssl_ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif

    SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_PEER, ion_ssl_verify_cb);

    SSL_CTX_set_options(ssl->ctx, ssl_ctx_options);
    SSL_CTX_set_session_id_context(ssl->ctx, (unsigned char *)(void *)ssl->ctx, sizeof(ssl->ctx));

    return Z_OBJ(zssl);
}

CLASS_METHOD(ION_SSL, server) {
    zend_object * object;
    zend_long     crypt_method = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(crypt_method)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    object = ion_ssl_factory(crypt_method);
    if(object) {
        RETURN_OBJ(object);
    }
}

METHOD_ARGS_BEGIN(ION_SSL, server, 0)
    METHOD_ARG_LONG(method, 0)
METHOD_ARGS_END()

CLASS_METHOD(ION_SSL, client) {
    zend_object * object;
    zend_long     crypt_method = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(crypt_method)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    object = ion_ssl_factory(crypt_method | ION_SSL_IS_CLIENT);
    if(object) {
        RETURN_OBJ(object);
    }
}

METHOD_ARGS_BEGIN(ION_SSL, client, 0)
    METHOD_ARG_LONG(method, 0)
METHOD_ARGS_END()

CLASS_METHOD(ION_SSL, __construct) {}

METHOD_WITHOUT_ARGS(ION_SSL, __construct)

CLASS_METHOD(ION_SSL, ticket) {
    zend_bool   status = 0;
    ion_ssl   * ssl = get_this_instance(ion_ssl);

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

METHOD_ARGS_BEGIN(ION_SSL, ticket, 0)
    METHOD_ARG_BOOL(status, 0)
METHOD_ARGS_END()

CLASS_METHOD(ION_SSL, compression) {
    zend_bool   status = 0;
    ion_ssl   * ssl = get_this_instance(ion_ssl);

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

METHOD_ARGS_BEGIN(ION_SSL, compression, 0)
    METHOD_ARG_BOOL(status, 0)
METHOD_ARGS_END()

/* public function ION\SSL::verifyPeer(bool $status = true) : self */
CLASS_METHOD(ION_SSL, verifyPeer) {
    zend_bool   status = 0;
    ion_ssl   * ssl = get_this_instance(ion_ssl);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(status)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(status) {
        SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_PEER, ion_ssl_verify_cb);
    } else {
        SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_NONE, NULL);
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_SSL, verifyPeer, 0)
    METHOD_ARG_BOOL(status, 0)
METHOD_ARGS_END()

/* public function ION\SSL::verifyDepth(int $depth = -1) : self */
CLASS_METHOD(ION_SSL, verifyDepth) {
    zend_long   depth = -1;
    ion_ssl   * ssl = get_this_instance(ion_ssl);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(depth)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    ssl->verify_depth = (int)depth;

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_SSL, verifyDepth, 0)
    METHOD_ARG_BOOL(depth, 0)
METHOD_ARGS_END()

/* public function ION\SSL::localCert(string $local_cert, string $local_pk = null) : self */
CLASS_METHOD(ION_SSL, localCert) {
    zend_string * local_cert = NULL;
    zend_string * local_pk = NULL;
    ion_ssl   * ssl = get_this_instance(ion_ssl);

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_STR(local_cert)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_EX(local_pk, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    char resolved_path_buff[MAXPATHLEN];

    if (VCWD_REALPATH(local_cert->val, resolved_path_buff)) {
        /* a certificate to use for authentication */
        if (SSL_CTX_use_certificate_chain_file(ssl->ctx, resolved_path_buff) != 1) {
            zend_throw_exception_ex(ion_ce_ION_SSLException, 0,
                "Unable to set local cert chain file '%s'; Check that your cafile/capath settings include details of your certificate and its issuer", local_cert->val);
            return;
        }
        if (local_pk) {
            char resolved_path_buff_pk[MAXPATHLEN];
            if (VCWD_REALPATH(local_pk->val, resolved_path_buff_pk)) {
                if (SSL_CTX_use_PrivateKey_file(ssl->ctx, resolved_path_buff_pk, SSL_FILETYPE_PEM) != 1) {
                    zend_throw_exception_ex(ion_ce_ION_SSLException,0,
                        "Unable to set private key file '%s'", resolved_path_buff_pk);
                    return;
                }
            }
        } else {
            if (SSL_CTX_use_PrivateKey_file(ssl->ctx, resolved_path_buff, SSL_FILETYPE_PEM) != 1) {
                zend_throw_exception_ex(ion_ce_ION_SSLException, 0,
                    "Unable to set private key file '%s'", resolved_path_buff);
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
            zend_throw_exception(ion_ce_ION_SSLException, "Private key does not match certificate!", 0);
            return;
        }
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_SSL, localCert, 1)
    METHOD_ARG_STRING(local_cert, 0)
    METHOD_ARG_STRING(local_pk, 0)
METHOD_ARGS_END()

/* public function ION\SSL::passPhrase(string $phrase) : self */
CLASS_METHOD(ION_SSL, passPhrase) {
    zend_string * phrase = NULL;
    ion_ssl     * ssl = get_this_instance(ion_ssl);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(phrase)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    SSL_CTX_set_default_passwd_cb_userdata(ssl->ctx, ssl);
    SSL_CTX_set_default_passwd_cb(ssl->ctx, ion_ssl_passwd_cb);
    ssl->passphrase = zend_string_copy(phrase);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_SSL, passPhrase, 1)
    METHOD_ARG_STRING(phrase, 0)
METHOD_ARGS_END()

/* public function ION\SSL::allowSelfSigned(bool $state = true) : self */
CLASS_METHOD(ION_SSL, allowSelfSigned) {
    zend_bool   state = true;
    ion_ssl   * ssl = get_this_instance(ion_ssl);

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(state)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(state) {
        ssl->flags |= ION_SSL_ALLOW_SELF_SIGNED;
    } else {
        ssl->flags &= ~ION_SSL_ALLOW_SELF_SIGNED;
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_SSL, allowSelfSigned, 0)
    METHOD_ARG_BOOL(state, 0)
METHOD_ARGS_END()

/* public function ION\SSL::ca(string $cafile, string $path) : self */
CLASS_METHOD(ION_SSL, ca) {
    zend_string * cafile = NULL;
    zend_string * capath = NULL;
    char * cafile_str = NULL;
    char * capath_str = NULL;
    ion_ssl     * ssl = get_this_instance(ion_ssl);

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_EX(cafile, 1, 0)
        Z_PARAM_STR_EX(capath, 1, 0)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(cafile) {
        cafile_str = cafile->val;
        if(!(ssl->flags & ION_SSL_IS_CLIENT)) {
            /* Servers need to load and assign CA names from the cafile */
            STACK_OF(X509_NAME) *cert_names = SSL_load_client_CA_file(cafile->val);
            if (cert_names != NULL) {
                SSL_CTX_set_client_CA_list(ssl->ctx, cert_names);
            } else {
                zend_throw_exception_ex(ion_ce_ION_SSLException, 0, "Failed loading CA names from cafile %s", cafile->val);
                return;
            }
        }
    }

    if(capath) {
        capath_str = capath->val;
    }

    if (cafile || capath) {
        if (!SSL_CTX_load_verify_locations(ssl->ctx, cafile_str, capath_str)) {
            zend_throw_exception_ex(ion_ce_ION_SSLException, 0, "Unable to load verify locations %s, %s", cafile_str, capath_str);
            return;
        }
    } else {
        if ((ssl->flags & ION_SSL_IS_CLIENT) && !SSL_CTX_set_default_verify_paths(ssl->ctx)) {
            zend_throw_exception(ion_ce_ION_SSLException,
                             "Unable to set default verify locations", 0);
            return;
        }
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_SSL, ca, 1)
    METHOD_ARG_STRING(cafile, 0)
    METHOD_ARG_STRING(capath, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_SSL)
    METHOD(ION_SSL, server,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_SSL, client,          ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_SSL, __construct,     ZEND_ACC_PRIVATE)
    METHOD(ION_SSL, ticket,          ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, compression,     ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, verifyPeer,      ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, verifyDepth,     ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, localCert,       ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, passPhrase,      ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, allowSelfSigned, ZEND_ACC_PUBLIC)
    METHOD(ION_SSL, ca,              ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_SSL) {

    SSL_load_error_strings();
    SSL_library_init();
    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll()) {
        zend_error(E_ERROR, "SSL: failed to generate entropy");
        return FAILURE;
    }

    GION(ssl_index) = SSL_get_ex_new_index(0, "PHP ION index", NULL, NULL, NULL);

    pion_register_class(ION_SSL, "ION\\SSL", ion_ssl_init, CLASS_METHODS(ION_SSL));
    pion_init_std_object_handlers(ION_SSL);
    pion_set_object_handler(ION_SSL, free_obj, ion_ssl_free);
    pion_set_object_handler(ION_SSL, clone_obj, NULL);

    PION_CLASS_CONST_LONG(ION_SSL, "METHOD_AUTO",  ION_SSL_CRYPTO_METHOD_AUTO);

    PION_CLASS_CONST_LONG(ION_SSL, "METHOD_SSLv2",  ION_SSL_CRYPTO_METHOD_SSLv2);
    PION_CLASS_CONST_LONG(ION_SSL, "METHOD_SSLv3",  ION_SSL_CRYPTO_METHOD_SSLv3);
    PION_CLASS_CONST_LONG(ION_SSL, "METHOD_TLSv10",   ION_SSL_CRYPTO_METHOD_TLSv10);
    PION_CLASS_CONST_LONG(ION_SSL, "METHOD_TLSv11",   ION_SSL_CRYPTO_METHOD_TLSv11);
    PION_CLASS_CONST_LONG(ION_SSL, "METHOD_TLSv12",   ION_SSL_CRYPTO_METHOD_TLSv12);

    PION_REGISTER_VOID_EXTENDED_CLASS(ION_SSLException, ion_ce_ION_RuntimeException, "ION\\SSLException");


    return SUCCESS;
}
