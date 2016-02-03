#include "ion.h"

#include "php_ion.h"
#include <ext/standard/info.h>

ZEND_DECLARE_MODULE_GLOBALS(ion);

#ifdef COMPILE_DL_ION
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(ion);
#endif


static const zend_module_dep ion_depends[] = {
    ZEND_MOD_REQUIRED("spl")
    ZEND_MOD_REQUIRED("openssl")
    {NULL, NULL, NULL}
};

static void _engine_log(int severity, const char *msg) {
    switch (severity) {
        case _EVENT_LOG_ERR:
            zend_error(E_ERROR, "Libevent error: %s", msg);
            break;
        case _EVENT_LOG_MSG:
            zend_error(E_NOTICE, "Libevent notice: %s", msg);
            break;
        case _EVENT_LOG_DEBUG:
            zend_error(E_NOTICE, "Libevent debug: %s", msg);
            break;
        case _EVENT_LOG_WARN:
        default:
            zend_error(E_WARNING, "Libevent warning: %s", msg);
            break;
    }
}

static void _engine_fatal(int err) {
    zend_error(E_CORE_ERROR, "Libevent fatal error: %s", strerror(err));
    _exit(err);
}

static PHP_GINIT_FUNCTION(ion) {
#if defined(COMPILE_DL_ION) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    memset(ion_globals, 0, sizeof(*ion_globals));
    ion_globals->cache = pecalloc(1, sizeof(zend_ion_global_cache), 1);
    ion_interned_strings_ctor();

}

static PHP_GSHUTDOWN_FUNCTION(ion) {
#if defined(COMPILE_DL_ION) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    ion_interned_strings_dtor();
    pefree(ion_globals->cache, 1);
}

/* Init module callback */
PHP_MINIT_FUNCTION(ion) {
    event_set_mem_functions(php_emalloc_wrapper, php_realloc_wrapper, php_efree_wrapper);
    event_set_fatal_callback(_engine_fatal);
    event_set_log_callback(_engine_log);

    GION(base) = event_base_new();

    event_base_priority_init(GION(base), ION_MAX_PRIORITY);


    STARTUP_MODULE(exceptions);
    STARTUP_MODULE(ION_Debug);
    STARTUP_MODULE(promisor);
    STARTUP_MODULE(ION_Promise);
    STARTUP_MODULE(ION_ResolvablePromise);
    STARTUP_MODULE(ION_Deferred);
    STARTUP_MODULE(ION_Sequence);
    STARTUP_MODULE(ION);
    STARTUP_MODULE(ION_Crypto);
    STARTUP_MODULE(ION_DNS);
    STARTUP_MODULE(ION_FS);
    STARTUP_MODULE(ION_Listener);
    STARTUP_MODULE(ION_Stream);
    STARTUP_MODULE(ION_Process);
    STARTUP_MODULE(ION_URI);
    STARTUP_MODULE(ION_Stream_StorageAbstract);
    STARTUP_MODULE(ION_Stream_Storage);
    STARTUP_MODULE(ION_Stream_Server);
    STARTUP_MODULE(ION_Stream_Client);
    STARTUP_MODULE(ION_HTTP_Message);
    STARTUP_MODULE(ION_HTTP_Request);
    STARTUP_MODULE(ION_HTTP_Response);
    STARTUP_MODULE(ION_HTTP_WebSocket_Frame);
    STARTUP_MODULE(ION_HTTP);

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

    if(GION(define_metrics)) {
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
    }

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ion) {
    SHUTDOWN_MODULE(ION_Process);
    SHUTDOWN_MODULE(ION_FS);
    SHUTDOWN_MODULE(ION_DNS);
    SHUTDOWN_MODULE(ION_Deferred);
    SHUTDOWN_MODULE(promisor);
    SHUTDOWN_MODULE(ION_Crypto);
    SHUTDOWN_MODULE(ION_Stream);
    SHUTDOWN_MODULE(ION);


    event_base_free( GION(base) );

    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

/* Start SAPI request */
PHP_RINIT_FUNCTION(ion) {
#if defined(COMPILE_DL_ION) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    ACTIVATE_MODULE(promisor);
    ACTIVATE_MODULE(ION);
    ACTIVATE_MODULE(ION_Stream);
    ACTIVATE_MODULE(ION_DNS);
    ACTIVATE_MODULE(ION_FS);
    ACTIVATE_MODULE(ION_Process);
    return SUCCESS;
}

/* End SAPI request */
PHP_RSHUTDOWN_FUNCTION(ion) {
    DEACTIVATE_MODULE(ION_Process);
    DEACTIVATE_MODULE(ION_FS);
    DEACTIVATE_MODULE(ION_DNS);
    DEACTIVATE_MODULE(ION_Stream);
    DEACTIVATE_MODULE(ION);
    DEACTIVATE_MODULE(promisor);
    return SUCCESS;
}


PHP_MINFO_FUNCTION(ion) {
    struct event_base *base = event_base_new();
    event_set_mem_functions(php_emalloc_wrapper, php_realloc_wrapper, php_efree_wrapper);
    int features = event_base_get_features(base);
    char * http_version;
    spprintf(&http_version, 16, "%d.%d.%d", HTTP_PARSER_VERSION_MAJOR, HTTP_PARSER_VERSION_MINOR, HTTP_PARSER_VERSION_PATCH);

    php_info_print_table_start();
    php_info_print_table_header(2, "ion", "enabled");
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
    php_info_print_table_row(2, "ion.dns.support", "yes");
    php_info_print_table_row(2, "ion.ssl.support", "yes");
    php_info_print_table_row(2, "ion.http.version", http_version);
    php_info_print_table_end();

    event_base_free(base);
    efree(http_version);
}


/* ION module meta */
zend_module_entry ion_module_entry = {
        STANDARD_MODULE_HEADER_EX,
        NULL,
        ion_depends, // depends
        "ion", // module name
        NULL,
        PHP_MINIT(ion), // module init callback
        PHP_MSHUTDOWN(ion),  // module shutdown callback
        PHP_RINIT(ion),  // request init callback
        PHP_RSHUTDOWN(ion), // request shutdown callback
        PHP_MINFO(ion), // module info callback
        ION_VERSION, // module version
        PHP_MODULE_GLOBALS(ion),
        PHP_GINIT(ion),
        PHP_GSHUTDOWN(ion),
        NULL,
        STANDARD_MODULE_PROPERTIES_EX
};