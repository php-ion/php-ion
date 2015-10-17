#ifndef PION_ENGINE_H
#define PION_ENGINE_H

#include <ext/spl/spl_functions.h>
#include <Zend/zend_generators.h>

#ifndef zend_uint
#define zend_uint uint32_t
#endif

#define ion_get_class(class_name) _ion_get_class_ ## class_name()

#define _ion_get_class_Generator() zend_ce_generator;

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
    zend_object_handlers h ## cls; \
    zend_class_entry * _ion_get_class_ ## cls() {       \
        return c ## cls;             \
    }


#define DEFINE_CLASS(class) ION_DEFINE_CLASS(class);

#define get_object_instance(obj, type) \
    (type *)(obj)
//    (type *)((char*)(obj) - XtOffsetOf(type, std))

#define get_instance(pz, type) \
    get_object_instance(Z_OBJ_P(pz), type)

#define get_this_instance(type) \
    get_instance(getThis(), type)

#define Z_REFLECTION_P(zv)  reflection_object_from_obj(Z_OBJ_P((zv)))

#define getThisInstance()             zend_object_store_get_object(this_ptr TSRMLS_CC)
#define getThisInstanceEx(obj_type)   (obj_type) zend_object_store_get_object(this_ptr TSRMLS_CC)

#define PARSE_ARGS(format, ...)                                                 \
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, ##__VA_ARGS__) == FAILURE) {    \
        return;                                                                 \
    }

#define CLASS_METHODS_START(class_name) \
    static const zend_function_entry m ## class_name[] = {

#define CLASS_METHODS_END \
        {NULL, NULL, NULL} \
    }

#define RETURN_THIS()                           \
    if(return_value_used) {                     \
        RETVAL_ZVAL(this_ptr, 1, 0);            \
    }                                           \
    return;

/**
 * Define instance destructor function
 */
#define CLASS_INSTANCE_FREE(class) \
    static void _ ## class ## _free(zend_object * object TSRMLS_DC)

//#define getInstanceObject(obj_type)   ((obj_type) object);

/**
 * Define instance constructor function
 */
#define CLASS_INSTANCE_INIT(class) \
    static zend_object * _ ## class ## _init(zend_class_entry * ce TSRMLS_DC)


#define getInstance(zobj)   zend_object_store_get_object(zobj TSRMLS_CC)

#define emalloc_instance(type) ecalloc(1, sizeof(type) + zend_object_properties_size(ce))

#define RETURN_INSTANCE(class, object) \
    object->std.ce = ce; \
    zend_object_std_init(&object->std, ce); \
    object_properties_init(&object->std, ce); \
    object->std.handlers = &h ## class; \
    return &object->std;

#define _PION_REGISTER_CLASS(class, class_name)                                    \
    spl_register_std_class(&c ## class, class_name, _ ## class ## Ctor, m ## class TSRMLS_CC);   \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define PION_REGISTER_CLASS(class, class_name)     \
    pion_register_std_class(&c ## class, class_name, _ ## class ## _init, m ## class TSRMLS_CC);   \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers)); \
    h ## class.free_obj = _ ## class ## _free;

#define PION_REGISTER_PLAIN_CLASS(class, class_name)                                    \
    spl_register_std_class(&c ## class, class_name, NULL, m ## class TSRMLS_CC);   \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define PION_REGISTER_EXTENDED_CLASS_WITH_CTOR(class, parent_class, class_name) \
    spl_register_sub_class(&c ## class, c ## parent_class, class_name, _ ## parent_class ## Ctor, m ## class TSRMLS_CC); \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define REGISTER_EXTENDED_CLASS(class, parent_class, class_name) \
    spl_register_sub_class(&c ## class, ion_get_class(parent_class), class_name, NULL, m ## class TSRMLS_CC); \
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

#define METHOD_ARGS_BEGIN_RETURN_INT(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_LONG, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_CALLABLE(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args, IS_CALLABLE, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_STRING(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args, IS_STRING, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_BOOL(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args, IS_STRING, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_OBJECT(class_name, method_name, required_num_args, return_class_name, allow_null) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args, IS_OBJECT, return_class_name, allow_null)

#define METHOD_ARGS_BEGIN_RETURN_ARRAY(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args, IS_ARRAY, NULL, 0)

#define METHOD_ARG(name, pass_by_ref)             ZEND_ARG_INFO(pass_by_ref, name)
#define METHOD_ARG_LONG(name, pass_by_ref)        METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_STRING(name, pass_by_ref)      METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_DOUBLE(name, pass_by_ref)      METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_FLOAT(name, pass_by_ref)       METHOD_ARG_DOUBLE(name, pass_by_ref)
#define METHOD_ARG_BOOL(name, pass_by_ref)        METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_RESOURCE(name, pass_by_ref)    METHOD_ARG(name, pass_by_ref)
#define METHOD_ARG_ARRAY(name, pass_by_ref)       ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_ARRAY, allow_null)
#define METHOD_ARG_CALLBACK(name, pass_by_ref, allow_null)            ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_CALLABLE, allow_null)
#define METHOD_ARG_TYPE(name, type_hint, allow_null, pass_by_ref)     ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)
#define METHOD_ARG_OBJECT(name, classname, allow_null, pass_by_ref)   ZEND_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null)

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

#ifdef PHP7
#define  ZEND_PARSE_PARAMETERS_END_THROW(ION_InvaliArgumentException)
#else
#define  ZEND_PARSE_PARAMETERS_END_THROW(ION_InvalidArgumentException)  \
            ZEND_PARSE_PARAMETERS_END(pion_throw_invalid_argument_exception(""))
#endif
#define PION_INI_BEGIN(module)		static const zend_ini_entry ini_entries[] = {
#define PION_INI_END()		{ 0, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0, 0, NULL } };


PHPAPI void pion_register_interface(zend_class_entry ** ppce, char * class_name, const zend_function_entry * functions);
PHPAPI void pion_register_std_class(zend_class_entry ** ppce, char * class_name, void * obj_ctor, const zend_function_entry * function_list);
PHPAPI void pion_register_sub_class(zend_class_entry ** ppce, zend_class_entry * parent_ce, char * class_name, void *obj_ctor, const zend_function_entry * function_list);

#endif //PION_ENGINE_H
