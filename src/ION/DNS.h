#ifndef ION_DNS_H
#define ION_DNS_H

#include "../pion.h"

BEGIN_EXTERN_C();

#define ION_DNS_RESOLV_CONF_DEFAULT "/etc/resolv.conf"

#define ION_DNS_ADDR_IPV4 1
#define ION_DNS_ADDR_IPV6 2
#define ION_DNS_ADDR_IP_ANY (ION_DNS_ADDR_IPV4 | ION_DNS_ADDR_IPV6)

typedef struct _ion_dns_addr_request {
    struct evdns_getaddrinfo_request * request;
    char * domain;
    zval * deferred;
} ion_dns_addr_request;

ZEND_BEGIN_MODULE_GLOBALS(ION_DNS)
    char          * resolv_conf;
ZEND_END_MODULE_GLOBALS(ION_DNS)

DEFINE_CLASS(ION_DNS);

END_EXTERN_C();

#endif //ION_DNS_H

