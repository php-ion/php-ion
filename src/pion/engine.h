#ifndef PION_ENGINE_H
#define PION_ENGINE_H

#include <ext/spl/spl_functions.h>

#define CE(class) \
    c ## class

#define DEFINE_CLASS(class) \
    zend_class_entry *c ## class; \
    zend_object_handlers h ## class;

#define getThisInstance(obj_type)   (obj_type) zend_object_store_get_object(this_ptr TSRMLS_CC)
#define getThisInstanceVoid()   zend_object_store_get_object(this_ptr TSRMLS_CC)

#define PARSE_ARGS(format, ...)                                                 \
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, ##__VA_ARGS__) == FAILURE) {    \
        return;                                                                 \
    }

#define NOT_ALLOWED_NULL 0
#define ALLOWED_NULL 1

#define IS_NOT_REF  0
#define IS_REF  1

#define CLASS_METHODS_START(class_name) \
    static const zend_function_entry m ## class_name[] = {

#define CLASS_METHODS_END \
        {NULL, NULL, NULL} \
    }

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



/**
 * Define instance destructor function
 */
#define CLASS_INSTANCE_DTOR(class) \
    static void _ ## class ## Dtor(void *object TSRMLS_DC)

#define getInstanceObject(obj_type)   ((obj_type) object);

/**
 * Define instance constructor function
 */
#define CLASS_INSTANCE_CTOR(class) \
    static zend_object_value _ ## class ## Ctor(zend_class_entry *ce TSRMLS_DC)


#define RETURN_INSTANCE(class, object) \
    zend_object_value instance;   \
    zend_object_std_init(&(object->std), c ## class TSRMLS_CC); \
    object_properties_init(&object->std, c ## class); \
    instance.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) _ ## class ## Dtor, NULL TSRMLS_CC); \
    instance.handlers = &h ## class; \
    return instance;

#define PION_REGISTER_CLASS(class, class_name)                                    \
    spl_register_std_class(&c ## class, class_name, _ ## class ## Ctor, m ## class TSRMLS_CC);   \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define CLASS_METHOD(class, method, acc_flags) \
    static const zend_uint flags ## class ## method = acc_flags; \
    PHP_METHOD(class, method)

#define METHOD_WITHOUT_ARGS(class_name, method_name) \
    METHOD_ARGS_BEGIN(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()

#define METHOD_ARGS_BEGIN(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args)

#define METHOD_ARG(name, pass_by_ref)  ZEND_ARG_INFO(pass_by_ref, name)

#define METHOD_ARG_TYPE(name, type_hint, allow_null, pass_by_ref)     ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)

#define METHOD_ARGS_END() \
    ZEND_END_ARG_INFO()

#define ARG_TYPE(name, type_hint, allow_null) \
    ZEND_ARG_TYPE_INFO(0, name, type_hint, allow_null)

#define METHOD(class_name, method_name) \
    ZEND_ME(class_name, method_name, args ## class_name ## method_name, flags ## class_name ## method_name)



#endif //PION_ENGINE_H
