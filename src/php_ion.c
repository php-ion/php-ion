/*  */
#include "php.h"
#include "ext/spl/spl_functions.h"
#include "ext/standard/php_var.h"
#include "ext/standard/info.h"

#ifdef ZTS
#  include "TSRM.h"
#endif

/* Libevent */
#include <event.h>

#include "php_ion.h"
#include "pion.h"

extern ion_base *ionBase;

#ifdef COMPILE_DL_ION
ZEND_GET_MODULE(ion);
#endif

static const zend_module_dep ion_depends[] = {
    ZEND_MOD_REQUIRED("SPL")
    {NULL, NULL, NULL}
};


#ifdef ION_DEBUG

ZEND_API void * _php_emalloc(size_t size) {
    return emalloc(size);
}

ZEND_API void * _php_realloc(void * nmemb, size_t size) {
    return erealloc(nmemb, size);
}

ZEND_API void _php_efree(void * ptr) {
    efree(ptr);
}

#endif

static void _engine_log(int severity, const char *msg) {
    switch (severity) {
        case _EVENT_LOG_ERR:
            zend_error(E_ERROR, "Libevent: %s", msg);
            break;
        case _EVENT_LOG_MSG:
            zend_error(E_NOTICE, "Libevent: %s", msg);
            break;
        case _EVENT_LOG_DEBUG:
            zend_error(E_NOTICE, "Libevent: %s", msg);
            break;
        case _EVENT_LOG_WARN:
        default:
            zend_error(E_WARNING, "Libevent: %s", msg);
            break;
    }
}

static void _engine_fatal(int err) {
    zend_error(E_CORE_ERROR, "Libevent fatal error: %s", strerror(err));
    _exit(err);
}

/* ION module meta */
zend_module_entry ion_module_entry = {
        STANDARD_MODULE_HEADER_EX,
        NULL,
        ion_depends, // depends
        "ion", // module name
        NULL,
        PHP_MINIT(ion), // module init callback
        NULL,  // module shutdown callback
        PHP_RINIT(ion),  // request init callback
        PHP_RSHUTDOWN(ion), // request shutdown callback
        PHP_MINFO(ion), // module info callback
        ION_VERSION, // module version
        STANDARD_MODULE_PROPERTIES
};

/* Init module callback */
PHP_MINIT_FUNCTION(ion) {
    event_set_log_callback(_engine_log);
    event_set_fatal_callback(_engine_fatal);

//    ionBase            = pemalloc(sizeof(ion_base), 1);
//    memset(ionBase, 0, sizeof(ion_base));
//    ION(i)             = 1;
//    ION(base)          = event_base_new();

    STARTUP_MODULE(ION_Debug);
    STARTUP_MODULE(ION_Data_LinkedList);
//    STARTUP_MODULE(ION_Data_SkipList);
//    STARTUP_MODULE(ION_Deferred);
//    STARTUP_MODULE(ION_Promise_Result);
//    STARTUP_MODULE(ION_Promise);
//    STARTUP_MODULE(ION_ResolvablePromise);
//    STARTUP_MODULE(ION);
//    STARTUP_MODULE(ION_DNS);
//    STARTUP_MODULE(ION_Process);
//    STARTUP_MODULE(ION_Stream);

#ifdef ION_DEBUG
    event_set_mem_functions(_php_emalloc, _php_realloc, _php_efree);
#endif

    long KB = 1000;
    long MB = 1000 * KB;
    long GB = 1000 * MB;
    long TB = 1000 * GB;
    long PB = 1000 * TB;

    long KiB = 1024;
    long MiB = 1024 * KiB;
    long GiB = 1024 * MiB;
    long TiB = 1024 * GiB;
    long PiB = 1024 * TiB;

    REGISTER_LONG_CONSTANT("B", 1,       CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KiB", KiB,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MiB", MiB,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GiB", GiB,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TiB", TiB,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("PiB", PiB,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KB", KB,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MB", MB,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GB", GB,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TB", TB,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("PB", PB,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("Sec", 1,              CONST_CS | CONST_PERSISTENT);
    REGISTER_DOUBLE_CONSTANT("mSec", 1e-3,        CONST_CS | CONST_PERSISTENT);
    REGISTER_DOUBLE_CONSTANT("uSec", 1e-6,        CONST_CS | CONST_PERSISTENT);
    REGISTER_DOUBLE_CONSTANT("nSec", 1e-9,        CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("Min", 60,             CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("Hour", 60 * 60,       CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("Hours", 60 * 60,      CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("Day", 24 * 60 * 60,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("Days", 24 * 60 * 60,  CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ion) {
    SHUTDOWN_MODULE(ION_Debug);
    SHUTDOWN_MODULE(ION_Data_LinkedList);
//    SHUTDOWN_MODULE(ION_Data_SkipList);
//    SHUTDOWN_MODULE(ION_Process);
//    SHUTDOWN_MODULE(ION_Stream);
//    SHUTDOWN_MODULE(ION_DNS);
//    SHUTDOWN_MODULE(ION_ResolvablePromise);
//    SHUTDOWN_MODULE(ION_Promise);
//    SHUTDOWN_MODULE(ION_Promise_Result);
//    SHUTDOWN_MODULE(ION_Deferred);
//    SHUTDOWN_MODULE(ION);

//    event_base_free( ION(base) );
//    pefree(ionBase, 1);

//    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

/* Start SAPI request */
PHP_RINIT_FUNCTION(ion) {
//    ACTIVATE_MODULE(ION_DNS);
//    ACTIVATE_MODULE(ION_Promise);
    return SUCCESS;
}

/* End SAPI request */
PHP_RSHUTDOWN_FUNCTION(ion) {
//    DEACTIVATE_MODULE(ION_Promise);
//    DEACTIVATE_MODULE(ION_DNS);
    return SUCCESS;
}


PHP_MINFO_FUNCTION(ion) {
    struct event_base *base = event_base_new();
    int features = event_base_get_features(base);

    php_info_print_table_start();
    php_info_print_table_header(2, "ion.support", "enabled");
    php_info_print_table_row(2, "ion.version", ION_VERSION);
    php_info_print_table_row(2, "ion.engine", ION_EVENT_ENGINE);
    php_info_print_table_row(2, "ion.engine.version", event_get_version());
    php_info_print_table_row(2, "ion.engine.builtin", "no");
    php_info_print_table_row(2, "ion.engine.method", event_base_get_method(base));
#ifdef _EVENT_HAVE_SENDFILE
    php_info_print_table_row(2, "ion.engine.sendfile", "sendfile");
#elif _EVENT_HAVE_MMAP
    php_info_print_table_row(2, "ion.engine.sendfile", "mmap");
#else
    php_info_print_table_row(2, "ion.engine.sendfile", "none");
#endif
    if(features & EV_FEATURE_ET) {
        php_info_print_table_row(2, "ion.engine.et_events", "yes");
    } else {
        php_info_print_table_row(2, "ion.engine.et_events", "no");

    }
    if(features & EV_FEATURE_O1) {
        php_info_print_table_row(2, "ion.engine.fast_method", "yes");
    } else {
        php_info_print_table_row(2, "ion.engine.fast_method", "no");
    }
    if(features & EV_FEATURE_FDS) {
        php_info_print_table_row(2, "ion.engine.fd_allowed", "yes");
    } else {
        php_info_print_table_row(2, "ion.engine.fd_allowed", "no");
    }
    php_info_print_table_row(2, "ion.dns.support", "enabled");
    php_info_print_table_row(2, "ion.ssl.support", "enabled");
    php_info_print_table_end();

    event_base_free(base);
}