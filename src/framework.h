
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

#define this_get_object()   zend_object_store_get_object(this_ptr TSRMLS_CC)

#define this_get_object_ex(obj_type)   ((obj_type) this_get_object())

#define PARSE_ARGS(format, ...)                                                 \
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, ##__VA_ARGS__) == FAILURE) {    \
        return;                                                                 \
    }

#define NOT_NULL 0
#define ALLOW_NULL 1

#define NOT_REF  0
#define IS_REF  1


#define CLASS_METHODS_START(class_name) \
    ZEND_BEGIN_ARG_INFO_EX(noargs_ ## class_name, 0, 0, 0) \
    ZEND_END_ARG_INFO(); \
    static const zend_function_entry m ## class_name[] = {

#define CLASS_METHODS_END \
        ZEND_ME_END \
    }

#define METHOD PHP_METHOD

#define METHOD_ARGS_LIST_BEGIN(class_name, method_name, required_num_args) ZEND_BEGIN_ARG_INFO_EX(args_ ## class_name ## _ ## method_name, 0, 0, required_num_args)

// method return reference
#define METHOD_REF_ARGS_LIST(class_name, method_name, required_num_args) ZEND_BEGIN_ARG_INFO_EX(arg ## class_name ## _ ## method_name, 0, return_ref, required_num_args)

#define METHOD_ARGS(class_name, method_name, flags) ZEND_ME(class_name, method_name, args_ ## class_name ## _ ## method_name, flags)
#define METHOD_NOARGS(class_name, method_name, flags) ZEND_ME(class_name, method_name, noargs_ ## class_name, flags)

#define METHOD_ARGS_LIST_END() ZEND_END_ARG_INFO()
#define METHOD_ARG(name, pass_by_ref)  ZEND_ARG_INFO(pass_by_ref, name)
#define METHOD_ARG_TYPE(name, type_hint, allow_null, pass_by_ref)     ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)


#define ARGUMENT(is_ref, name)  ZEND_ARG_INFO(is_ref, name)



#define ZEND_ME_END      {NULL, NULL, NULL}

#define ZEND_ME_ARG(class, method, flags)                \
    ZEND_ME(class, method, arginfo_ ## method, flags)


#define ZEND_ME_NOARG(class, method, flags)              \
    ZEND_ME(class, method, arginfo_noargs, flags)

#define REGISTER_CLASS(class, class_name, obj_ctor)                                    \
    spl_register_std_class(&c ## class, class_name, obj_ctor, m ## class TSRMLS_CC);   \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define RETURN_THIS()                           \
    if(return_value_used) {                     \
        RETVAL_ZVAL(this_ptr, 1, NULL);          \
    }                                           \
    return;


/* Fetch FD from ZVAL resource */
int php_stream_get_fd(zval *);

/**
 * For debug
 */

#define PHPDBG(msg, ...)    \
    printf("%s: ", __func__); \
    printf(msg, ##__VA_ARGS__); \
    printf("\n");

#define ZVAL_DUMP_PP(zvar)  ZVAL_DUMP_P(*zvar)

#define ZVAL_DUMP_P(zvar)  \
    php_printf("DUMP: "); \
    php_var_dump(&(zvar), 1 TSRMLS_CC);

#define ZVAL_DUMPF(zvar, format, ...)  \
    php_printf(format, ##__VA_ARGS__); \
    php_var_dump(&zvar, 1 TSRMLS_CC);


#endif //ION_FRAMEWORK_H
