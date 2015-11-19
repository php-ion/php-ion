#ifndef ION_DNS_H
#define ION_DNS_H

#include "../pion.h"

BEGIN_EXTERN_C();

#define ION_DNS_RESOLV_CONF_DEFAULT "/etc/resolv.conf"
#define ION_DNS_HOSTS_FILE_DEFAULT "/etc/hosts"

#define ION_DNS_RECORD_A       1
#define ION_DNS_RECORD_AAAA    2
#define ION_DNS_RECORDS_A_AAAA (ION_DNS_RECORD_A | ION_DNS_RECORD_AAAA)
#define ION_DNS_RECORD_CNAME   4
#define ION_DNS_ADDR_MASK      (ION_DNS_RECORD_A | ION_DNS_RECORD_AAAA)

#define ION_DNS_RECORD_BASE    (ION_DNS_RECORD_A | ION_DNS_RECORD_AAAA | ION_DNS_RECORD_CNAME)

typedef struct _ion_dns_addr_request {
    struct evdns_getaddrinfo_request * request;
    zend_string * domain;
    zend_object * deferred;
} ion_dns_addr_request;

END_EXTERN_C();

#endif //ION_DNS_H

