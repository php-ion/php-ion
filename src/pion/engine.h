#ifndef PION_ENGINE_H
#define PION_ENGINE_H

#include <ext/spl/spl_functions.h>

#define CE(class) \
    c ## class

#define ion_class_entry(cls) \
    c ## cls

#define ion_class_handlers(cls) \
    h ## cls

#define ion_define_class_entry(cls) \
    zend_class_entry *c ## class;

#define ion_define_class_handlers(cls) \
    zend_object_handlers h ## class;

#define ION_DEFINE_CLASS(cls)    \
    zend_class_entry *c ## cls; \
    zend_object_handlers h ## cls;

#define DEFINE_CLASS(class) \
    zend_class_entry *c ## class; \
    zend_object_handlers h ## class;

#define getThisInstance()             zend_object_store_get_object(this_ptr TSRMLS_CC)
#define getThisInstanceEx(obj_type)   (obj_type) zend_object_store_get_object(this_ptr TSRMLS_CC)

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

#define ALLOC_BOOL_ZVAL(var, bval)   \
    ALLOC_INIT_ZVAL(var);   \
    ZVAL_BOOL(var, bval);


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


#define getInstance(zobj)   zend_object_store_get_object(zobj TSRMLS_CC)


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

#define PION_REGISTER_PLAIN_CLASS(class, class_name)                                    \
    spl_register_std_class(&c ## class, class_name, NULL, m ## class TSRMLS_CC);   \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define REGISTER_VOID_EXTENDED_CLASS(class, parent_class, class_name, obj_ctor) \
    spl_register_sub_class(&c ## class, c ## parent_class, class_name, obj_ctor, NULL TSRMLS_CC); \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));


#define PION_CLASS_CONST_LONG(class, const_name, value) \
    zend_declare_class_constant_long(c ## class, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);

#define PION_CLASS_CONST_STRING(class, const_name, value) \
    zend_declare_class_constant_string(c ## class, const_name, sizeof(const_name)-1, value TSRMLS_CC);

#define CLASS_METHOD(class, method) \
    PHP_METHOD(class, method)

#define METHOD_WITHOUT_ARGS(class_name, method_name) \
    METHOD_ARGS_BEGIN(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()

#define METHOD_ARGS_BEGIN(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args)

#define METHOD_ARG(name, pass_by_ref)  ZEND_ARG_INFO(pass_by_ref, name)
#define METHOD_ARG_LONG(name, pass_by_ref)        METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_STRING(name, pass_by_ref)      METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_DOUBLE(name, pass_by_ref)      METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_FLOAT(name, pass_by_ref)       METHOD_ARG_DOUBLE(name, pass_by_ref)
#define METHOD_ARG_BOOL(name, pass_by_ref)        METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_RESOURCE(name, pass_by_ref)    METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_ARRAY(name, pass_by_ref)       METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_CALLBACK(name, pass_by_ref, allow_null)    METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_TYPE(name, type_hint, allow_null, pass_by_ref)     ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)
#define METHOD_ARG_OBJECT(name, classname, allow_null, pass_by_ref)     ZEND_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null)

#define METHOD_ARGS_END() \
    ZEND_END_ARG_INFO()

#define ARG_TYPE(name, type_hint, allow_null) \
    ZEND_ARG_TYPE_INFO(0, name, type_hint, allow_null)

#define METHOD(class_name, method_name, flags) \
    ZEND_ME(class_name, method_name, args ## class_name ## method_name, flags)

// for old php

#ifndef RETURN_ZVAL_FAST
#define RETVAL_ZVAL_FAST(z) do {      \
	zval *_z = (z);                   \
	if (Z_ISREF_P(_z)) {              \
		RETVAL_ZVAL(_z, 1, 0);        \
	} else {                          \
		zval_ptr_dtor(&return_value); \
		Z_ADDREF_P(_z);               \
		*return_value_ptr = _z;       \
	}                                 \
} while (0)
#define RETURN_ZVAL_FAST(z) { RETVAL_ZVAL_FAST(z); return; }
#endif

#endif //PION_ENGINE_H
