/*  */
#include "php.h"
#include "ext/spl/spl_functions.h"
#include "ext/standard/php_var.h"
#include "ext/standard/info.h"

#ifdef ZTS
#  include "TSRM.h"
#endif

//#ifdef HAVE_CONFIG_H
#include "config.h"
//#endif

/* Libevent */
#include <event.h>

#include "php_ion.h"
#include "pion.h"

extern IONBase *ionBase;

#define PION_FUNCTIONS


#ifdef COMPILE_DL_ION
ZEND_GET_MODULE(ion);
#endif

static const zend_module_dep ion_depends[] = {
    ZEND_MOD_REQUIRED("SPL")
    {NULL, NULL, NULL}
};

#ifdef PION_FUNCTIONS
// PION framework functions for testing

PHP_FUNCTION(pionCbCreate) {
    zval *zarg = NULL;
    zend_fcall_info        fci = empty_fcall_info;
    zend_fcall_info_cache  fcc = empty_fcall_info_cache;
    PARSE_ARGS("fz", &fci, &fcc, &zarg);
    pionCb *cb = pionCbCreate(&fci, &fcc TSRMLS_CC);
    int result = pionCbVoidWith1Arg(cb, zarg TSRMLS_CC);
    pionCbFree(cb);
    RETURN_LONG((long)result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_pionCbCreate, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(pionCbCreateFromZval) {
    zval *zarg = NULL;
    zval *zcb = NULL;
    PARSE_ARGS("zz", &zcb, &zarg);
    pionCb *cb = pionCbCreateFromZval(zcb TSRMLS_CC);
    int result = pionCbVoidWith1Arg(cb, zarg TSRMLS_CC);
    pionCbFree(cb);
    RETURN_LONG((long)result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_pionCbCreateFromZval, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

const zend_function_entry pion_functions[] = {
    PHP_FE(pionCbCreate, arginfo_pionCbCreate)
    PHP_FE(pionCbCreateFromZval, arginfo_pionCbCreateFromZval)
    {NULL, NULL, NULL}
};

#endif


/* ION module meta */
zend_module_entry ion_module_entry = {
        STANDARD_MODULE_HEADER_EX,
        NULL,
        ion_depends, // depends
        "ion", // module name
#ifdef PION_FUNCTIONS
        pion_functions, // module's functions
#else
        NULL,
#endif
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
    STARTUP_MODULE(ION_Data_LinkedList);
    STARTUP_MODULE(ION_Data_SkipList);
    STARTUP_MODULE(ION_Deferred);
    STARTUP_MODULE(ION);

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ion) {
    SHUTDOWN_MODULE(ION_Data_LinkedList);
    SHUTDOWN_MODULE(ION_Data_SkipList);
    SHUTDOWN_MODULE(ION_Deferred);
    SHUTDOWN_MODULE(ION);

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
    char engine[64], poll[32], available[16] = "";
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
    php_info_print_table_header(2, "ion.support", "enabled");
    php_info_print_table_row(2, "ion.version", PHP_ION_VERSION);
    php_info_print_table_row(2, "ion.engine", engine);
    php_info_print_table_row(2, "ion.engine.built-in", "no");
    php_info_print_table_row(2, "ion.dns.support", "enabled");
    php_info_print_table_row(2, "ion.dns.resolve.config", "/etc/resolve.conf");
    php_info_print_table_row(2, "ion.ssl.support", "disabled");
    php_info_print_table_row(2, "ion.event.poll", poll);
#ifdef _EVENT_HAVE_SENDFILE
    php_info_print_table_row(2, "ion.sendfile.engine", "sendfile");
#elif _EVENT_HAVE_MMAP
    php_info_print_table_row(2, "ion.sendfile.engine", "mmap");
#else
    php_info_print_table_row(2, "ion.sendfile.engine", "none");
#endif

    php_info_print_table_row(2, "ion.event.features", available);
    php_info_print_table_end();
}