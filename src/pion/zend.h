#ifndef PION_ZEND_H
#define PION_ZEND_H

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
#define this_object()   zend_object_store_get_object(this_ptr TSRMLS_CC)

#define this_get_object_ex(obj_type)   ((obj_type) this_get_object())
#define this_object_ex(obj_type)   ((obj_type) this_get_object())

#define PARSE_ARGS(format, ...)                                                 \
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, ##__VA_ARGS__) == FAILURE) {    \
        return;                                                                 \
    }

#define NOT_NULL 0
#define ALLOW_NULL 1

#define IS_NOT_REF  0
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


#define ALLOC_STRING_ZVAL(var, str, dup)     \
    ALLOC_INIT_ZVAL(var);   \
    ZVAL_STRING(var, str, dup);

#define ALLOC_STRINGL_ZVAL(var, str, len, dup)     \
    ALLOC_INIT_ZVAL(var);   \
    ZVAL_STRINGL(var, str, len, dup);

#define ALLOC_EMPTY_STRING_ZVAL(var)     \
    ALLOC_INIT_ZVAL(var);   \
    ZVAL_EMPTY_STRING(var);

#define ALLOC_LONG_ZVAL(var, num)   \
    ALLOC_INIT_ZVAL(var);   \
    ZVAL_LONG(var, num);



#endif //PION_ZEND_H
