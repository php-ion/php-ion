#include "DNS.h"
#include <event2/dns.h>

DEFINE_CLASS(ION_DNS);

/** public static function ION\DNS::getAddrInfo(string $domain, int $flags = 0) : Deferred */
CLASS_METHOD(ION_DNS, getAddrInfo) {

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
    ION(dns) = emalloc(sizeof(ion_dns));
    memset(ION(dns), 0, sizeof(ion_dns));

//    REGISTER_INI_ENTRIES();

    ION(dns)->evdns = evdns_base_new(ION(base), 1);
    evdns_base_resolv_conf_parse(ION(dns)->evdns, DNS_OPTIONS_ALL, ION_DNS_RESOLV_CONF_DEFAULT);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_DNS) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_DNS) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_DNS) {
    evdns_base_free(ION(dns)->evdns, 0);
    efree(ION(dns));

    return SUCCESS;
}