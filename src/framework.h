
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

/** main structure (class ION) */
typedef struct _ion_base {
    struct event_base *base;     // event base
    struct evdns_base *evdns;    // event DNS base
    struct event_config *config; // event config
//    zval *dns;                   // DNS instance
    long  i;                     // internal counter of timers
    HashTable *signals;          // array of listening signals
    HashTable *timers;           // array of timers
    HashTable *execs;            // array of process childs
    short has_fatals;            // flag, fatal error occured
    struct event *sigsegv;
//    LList *queue;                // queue of defers object
#ifdef ZTS
    void ***thread_ctx;
#endif
} IONBase;

#define ION(prop) \
    ionBase->prop

#define CE(class) \
    c ## class

#define DEFINE_CLASS(class) \
    zend_class_entry *c ## class; \
    zend_object_handlers h ## class;

#define OBJECT_INIT(retval, class, object, dtor) \
    zend_object_std_init(&(object->std), c ## class TSRMLS_CC); \
    object_properties_init(&object->std, c ## class); \
    retval.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) dtor, NULL TSRMLS_CC); \
    retval.handlers = &h ## class;

#define FETCH_HANDLE()   zend_object_store_get_object(this_ptr TSRMLS_CC)

#define PARSE_ARGS(format, ...)                                                 \
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, ##__VA_ARGS__) == FAILURE) {    \
        return;                                                                 \
    }

#define CLASS_METHODS_START(class) \
    static const zend_function_entry m ## class[] = {

#define CLASS_METHODS_END \
        ZEND_ME_END \
    }

#define ZEND_ME_END      {NULL, NULL, NULL}

#define ZEND_ME_ARG(class, method, flags) \
    ZEND_ME(class, method, arginfo_ ## method, flags)


#define ZEND_ME_NOARG(class, method, flags) \
    ZEND_ME(class, method, arginfo_noargs, flags)

#define REGISTER_CLASS(class, class_name, obj_ctor) \
    spl_register_std_class(&c ## class, class_name, obj_ctor, m ## class TSRMLS_CC); \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));


/* Fetch FD from ZVAL resource */
int php_stream_get_fd(zval *);

/**
 * For debug
 */

#define PHPDBG(msg, ...)    \
    printf("%s: ", __func__); \
    printf(msg, ##__VA_ARGS__); \
    printf("\n");

#endif //ION_FRAMEWORK_H
