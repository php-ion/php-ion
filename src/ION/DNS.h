#ifndef ION_DNS_H
#define ION_DNS_H

#include "../pion.h"

BEGIN_EXTERN_C();

#define ION_DNS_RESOLV_CONF_DEFAULT "/etc/resolv.conf"

ZEND_BEGIN_MODULE_GLOBALS(ION_DNS)
    char          * resolv_conf;
ZEND_END_MODULE_GLOBALS(ION_DNS)

DEFINE_CLASS(ION_DNS);

END_EXTERN_C();

#endif //ION_DNS_H

