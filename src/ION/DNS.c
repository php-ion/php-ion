#include "DNS.h"
#include <event2/dns.h>

DEFINE_CLASS(ION_DNS);

void _ion_dns_getaddrinfo_callback(int errcode, struct evutil_addrinfo *addr, void *arg) {
    ion_dns_addr_request *req = (ion_dns_addr_request *)arg;
    zval *result;
    if (!errcode) {
        ALLOC_INIT_ZVAL(result);
        array_init(result);
        struct evutil_addrinfo *ai;
        for (ai = addr; ai; ai = ai->ai_next) {
            char buf[128];
            const char *s = NULL;
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
            }
            if(s) {
                add_next_index_string(result, s, 1);
            }
        }
        evutil_freeaddrinfo(addr);
        ion_deferred_done(req->deferred, result);
    } else {
        result = pion_exception_new_ex(ION_RuntimeException, errcode, "DNS request failed: %s", evutil_gai_strerror(errcode));
        ion_deferred_fail(req->deferred, result);
    }
    zval_ptr_dtor(&result);
    zval_ptr_dtor(&req->deferred);
    efree(req->domain);
    efree(req);
}

void _ion_dns_getaddrinfo_cancel(zval * error, zval *zdeferred TSRMLS_DC) {
    ion_dns_addr_request *req = ion_deferred_store_get(zdeferred);
    evdns_getaddrinfo_cancel(req->request);
    zval_ptr_dtor(&req->deferred);
    efree(req->domain);
    efree(req);
}

void _ion_dns_clean_requests(void * r) {
    ion_dns_addr_request * req = (ion_dns_addr_request *) r;
    ion_deferred_free(req->deferred);
    evdns_getaddrinfo_cancel(req->request);
    efree(req->domain);
    efree(req);
}

/** public static function ION\DNS::getAddrInfo(string $domain, int $flags = self::ADDR_IP_ANY) : Deferred */
CLASS_METHOD(ION_DNS, getAddrInfo) {
    char * domain   = NULL;
    uint domain_len = 0;
    long flags      = ION_DNS_ADDR_IP_ANY;
    ion_dns_addr_request * req;
    struct evutil_addrinfo hints;


    PARSE_ARGS("s|l", &domain, &domain_len, &flags);
    if(zend_hash_find(ION(dns)->requests, domain, domain_len, (void **)&req) == SUCCESS) {
        RETURN_ZVAL(req->deferred, 1, 0);
    }

    memset(&hints, 0, sizeof(hints));
    if(flags & ION_DNS_ADDR_IPV4) {
        hints.ai_family = PF_INET;
    } else if(flags & ION_DNS_ADDR_IPV6) {
        hints.ai_family = PF_INET6;
    } else {
        hints.ai_family = PF_UNSPEC;
    }
    hints.ai_flags = EVUTIL_AI_CANONNAME;
    /* Unless we specify a socktype, we'll get at least two entries for
     * each address: one for TCP and one for UDP. That's not what we
     * want. */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    req = emalloc(sizeof(ion_dns_addr_request));
    memset(req, 0, sizeof(ion_dns_addr_request));
    if(zend_hash_add(ION(dns)->requests, domain, domain_len, req, sizeof(ion_dns_addr_request), NULL) == FAILURE) {
        ThrowRuntime("Failed to store request", -1);
        return;
    }
    req->domain = estrdup(domain);
    req->deferred = ion_deferred_new_ex(_ion_dns_getaddrinfo_cancel);
    ion_deferred_store(req->deferred, req, NULL);
    req->request = evdns_getaddrinfo(ION(dns)->evdns, domain, NULL, &hints, _ion_dns_getaddrinfo_callback, req);
    RETURN_ZVAL(req->deferred, 1, 0);
}

METHOD_ARGS_BEGIN(ION_DNS, getAddrInfo, 1)
    METHOD_ARG_STRING(domain, 0)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_DNS)
    METHOD(ION_DNS, getAddrInfo,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
CLASS_METHODS_END;

//ZEND_INI_BEGIN()
//    STD_PHP_INI_ENTRY("ion.dns.resolv_conf", ION_DNS_RESOLV_CONF_DEFAULT, PHP_INI_SYSTEM, OnUpdateStringUnempty, resolv_conf, ion_dns, ION(dns))
//ZEND_INI_END()

PHP_MINIT_FUNCTION(ION_DNS) {


    PION_REGISTER_PLAIN_CLASS(ION_DNS, "ION\\DNS");
    PION_CLASS_CONST_LONG(ION_DNS, "ARRD_IPV4", ION_DNS_ADDR_IPV4);
    PION_CLASS_CONST_LONG(ION_DNS, "ARRD_IPV6", ION_DNS_ADDR_IPV6);
    PION_CLASS_CONST_LONG(ION_DNS, "ARRD_IP_ANY", ION_DNS_ADDR_IP_ANY);
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
    efree(ION(dns));

    return SUCCESS;
}