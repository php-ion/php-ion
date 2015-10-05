#include "DNS.h"
#include <event2/dns.h>

DEFINE_CLASS(ION_DNS);

void _ion_dns_getaddrinfo_callback(int errcode, struct evutil_addrinfo * addr, void * arg) {
    ion_dns_addr_request * req = (ion_dns_addr_request *) arg;
    TSRMLS_FETCH_FROM_CTX(req->thread_ctx);
    zval * result, * A, * AAAA;
    if (!errcode) {
        ALLOC_INIT_ZVAL(result);
        ALLOC_INIT_ZVAL(A);
        ALLOC_INIT_ZVAL(AAAA);
        array_init(result);
        array_init(A);
        array_init(AAAA);
        add_assoc_string(result, "CNAME", req->domain, 1);
        add_assoc_zval(result, "A", A);
        add_assoc_zval(result, "AAAA", AAAA);
        struct evutil_addrinfo *ai;
        for (ai = addr; ai; ai = ai->ai_next) {
            char buf[128];
            const char *s = NULL;
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
                if(s) {
                    add_next_index_string(A, s, 1);
                }
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
                if(s) {
                    add_next_index_string(AAAA, s, 1);
                }
            }
        }
        evutil_freeaddrinfo(addr);
        ion_deferred_done(req->deferred, result);
    } else {
        result = pion_exception_new_ex(ion_get_class(ION_RuntimeException), errcode, "DNS request failed: %s", evutil_gai_strerror(errcode));
        ion_deferred_fail(req->deferred, result);
    }
    zval_ptr_dtor(&result);
    zval_ptr_dtor(&req->deferred);
    req->deferred = NULL;
    req->request = NULL;
    if(zend_hash_del(ION(dns)->requests, req->domain, req->domain_len + 1) == FAILURE) {
        zend_error(E_ERROR, "Failed to remove a addrinfo request");
    }
}

void _ion_dns_getaddrinfo_cancel(zval * error, zval *zdeferred TSRMLS_DC) {
    ion_dns_addr_request *req = ion_deferred_store_get(zdeferred);
    if(req->request) {
        evdns_getaddrinfo_cancel(req->request);
    }
    zval_ptr_dtor(&req->deferred);
    req->deferred = NULL;
    req->request = NULL;
    if(zend_hash_del(ION(dns)->requests, req->domain, req->domain_len + 1) == FAILURE) {
        zend_error(E_ERROR, "Failed to remove a addrinfo request");
    }
}

void _ion_dns_clean_requests(void * r) {
    ion_dns_addr_request * req = *(ion_dns_addr_request **) r;
    TSRMLS_FETCH_FROM_CTX(req->thread_ctx);
    if(req->deferred) {
        ion_deferred_free(req->deferred);
    }
    if(req->request) {
        evdns_getaddrinfo_cancel(req->request);
    }
    efree(req->domain);
    efree(req);
}

/** public static function ION\DNS::resolve(string $domain, int $flags = self::ADDR_IP_ANY) : Deferred */
CLASS_METHOD(ION_DNS, resolve) {
    char * domain   = NULL;
    uint domain_len = 0;
    long flags      = ION_DNS_RECORD_BASE;
    ion_dns_addr_request * req;
    ion_dns_addr_request ** found;
    struct evutil_addrinfo hints;


    PARSE_ARGS("s|l", &domain, &domain_len, &flags);
//    if (zend_symtable_exists(ION(dns)->requests, domain, domain_len + 1)) {
//        RETURN_TRUE;
//    }
    if(zend_hash_find(ION(dns)->requests, domain, domain_len + 1, (void **)&found) == SUCCESS) {
        RETURN_ZVAL((*found)->deferred, 1, 0);
    }

    memset(&hints, 0, sizeof(hints));
    if((flags & ION_DNS_RECORDS_A_AAAA) == ION_DNS_RECORDS_A_AAAA || (flags & ION_DNS_RECORDS_A_AAAA) == 0) {
        hints.ai_family = PF_UNSPEC;
    } else if(flags & ION_DNS_RECORD_A) {
        hints.ai_family = PF_INET;
    } else { // flags & ION_DNS_RECORD_AAAA
        hints.ai_family = PF_INET6;
    }
    hints.ai_flags = EVUTIL_AI_CANONNAME;
    /* Unless we specify a socktype, we'll get at least two entries for
     * each address: one for TCP and one for UDP. That's not what we
     * want. */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    req = emalloc(sizeof(ion_dns_addr_request));
    memset(req, 0, sizeof(ion_dns_addr_request));
    TSRMLS_SET_CTX(req->thread_ctx);
    req->domain = estrndup(domain, domain_len + 1);
    req->domain_len = domain_len;
    req->deferred = ion_deferred_new_ex(_ion_dns_getaddrinfo_cancel);
    if(zend_hash_add(ION(dns)->requests, domain, domain_len + 1, &req, sizeof(ion_dns_addr_request *), NULL) == FAILURE) {
        ThrowRuntime("Failed to store request", -1);
        return;
    }
    ion_deferred_store(req->deferred, req, NULL);
    req->request = evdns_getaddrinfo(ION(dns)->evdns, domain, NULL, &hints, _ion_dns_getaddrinfo_callback, req);
    RETURN_ZVAL(req->deferred, 1, 0);
}

METHOD_ARGS_BEGIN(ION_DNS, resolve, 1)
    METHOD_ARG_STRING(domain, 0)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_DNS)
    METHOD(ION_DNS, resolve,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

//ZEND_INI_BEGIN()
//    STD_PHP_INI_ENTRY("ion.dns.resolv_conf", ION_DNS_RESOLV_CONF_DEFAULT, PHP_INI_SYSTEM, OnUpdateStringUnempty, resolv_conf, ion_dns, ION(dns))
//ZEND_INI_END()

PHP_MINIT_FUNCTION(ION_DNS) {


    PION_REGISTER_PLAIN_CLASS(ION_DNS, "ION\\DNS");
    PION_CLASS_CONST_LONG(ION_DNS, "RECORD_A", ION_DNS_RECORD_A);
    PION_CLASS_CONST_LONG(ION_DNS, "RECORD_AAAA", ION_DNS_RECORD_AAAA);
    PION_CLASS_CONST_LONG(ION_DNS, "RECORD_CNAME", ION_DNS_RECORD_CNAME);
    ION(dns) = pemalloc(sizeof(ion_dns), 1);
    memset(ION(dns), 0, sizeof(ion_dns));
//    REGISTER_INI_ENTRIES();

    ION(dns)->evdns = evdns_base_new(ION(base), 1);
    evdns_base_resolv_conf_parse(ION(dns)->evdns, DNS_OPTIONS_ALL, ION_DNS_RESOLV_CONF_DEFAULT);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_DNS) {
    ALLOC_HASHTABLE(ION(dns)->requests);
    zend_hash_init(ION(dns)->requests, 128, NULL, _ion_dns_clean_requests, 0);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_DNS) {
    zend_hash_clean(ION(dns)->requests);
    zend_hash_destroy(ION(dns)->requests);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_DNS) {
    evdns_base_free(ION(dns)->evdns, 0);
    pefree(ION(dns), 1);

    return SUCCESS;
}