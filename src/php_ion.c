/*  */
#include "php.h"
#include "ext/spl/spl_functions.h"
#include "ext/standard/php_var.h"
#include "ext/standard/info.h"

#ifdef ZTS
#  include "TSRM.h"
#endif

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* Libevent */
#include <event.h>

#include "php_ion.h"
#include "framework.h"

IONBase *ionBase;

static const zend_module_dep ion_depends[] = {
        ZEND_MOD_REQUIRED("SPL")
        {NULL, NULL, NULL}
};

/* ION module meta */
zend_module_entry ion_module_entry = {
        STANDARD_MODULE_HEADER_EX,
        NULL,
        ion_depends, // depends
        "ION", // module name
        NULL, // module's functions
        PHP_MINIT(ion), // module init callback
        NULL,  // module shutdown callback
        PHP_RINIT(ion),  // request init callback
        PHP_RSHUTDOWN(ion), // request shutdown callback
        PHP_MINFO(ion), // module info callback
        PHP_ION_VERSION, // module version
        STANDARD_MODULE_PROPERTIES
};


/* Init module callback */
PHP_MINIT_FUNCTION(ion) {
    return SUCCESS;
}

/* Start SAPI request */
PHP_RINIT_FUNCTION(ion) {
    ionBase = emalloc(sizeof(IONBase));
    memset(ionBase, 0, sizeof(IONBase));
    ION(base)          = event_base_new();

    return SUCCESS;
}

/* End SAPI request */
PHP_RSHUTDOWN_FUNCTION(ion) {
    event_base_free( ION(base) );
    efree(ionBase);
    return SUCCESS;
}


PHP_MINFO_FUNCTION(ion) {
    char engine[64], poll[32], available[16];
    int features = 0;
    struct event_base *base = event_base_new();

    features = event_base_get_features(base);
    snprintf(engine, sizeof(engine) - 1, "libevent-%s", event_get_version());
    snprintf(poll, sizeof(poll) - 1, "%s", event_base_get_method(base));

    event_base_free(base);

    if(features & EV_FEATURE_ET) {
        strcat(available, "ET ");
    }

    if(features & EV_FEATURE_O1) {
        strcat(available, "O1 ");
    }

    if(features & EV_FEATURE_FDS) {
        strcat(available, "FDS ");
    }

    php_info_print_table_start();
    php_info_print_table_header(2, "ION support", "enabled");
    php_info_print_table_row(2, "ION version", PHP_ION_VERSION);
    php_info_print_table_row(2, "ION engine", engine);
    php_info_print_table_row(2, "DNS support", "enabled");
    php_info_print_table_row(2, "DNS resolve.conf file", "/etc/resolve.conf");
    php_info_print_table_row(2, "SSL support", "disabled");
    php_info_print_table_row(2, "Event poll", poll);
#ifdef _EVENT_HAVE_SENDFILE
    php_info_print_table_row(2, "Send file engine", "sendfile");
#elif _EVENT_HAVE_MMAP
    php_info_print_table_row(2, "Send file engine", "mmap");
#else
    php_info_print_table_row(2, "Send file engine", "none");
#endif

    php_info_print_table_row(2, "Features", available);
    php_info_print_table_end();

}