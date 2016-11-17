#ifndef PION_ENGINE_H
#define PION_ENGINE_H

#include <ext/spl/spl_functions.h>

#define ion_get_class(class_name) ion_ce_ ## class_name

#define ion_class_entry(class_name) ion_ce_ ## class_name
#define ion_object_handlers(class_name) ion_oh_ ## class_name

#define ion_class_handlers(cls) \
    h ## cls

#define ion_define_class_entry(cls) \
    zend_class_entry *c ## class;

#define ion_define_class_handlers(cls) \
    zend_object_handlers h ## class;

#define ION_DEFINE_CLASS(cls)    \
    zend_class_entry *ion_ce_ ## cls; \
    zend_object_handlers ion_oh_ ## cls;


#define DEFINE_CLASS(class) ION_DEFINE_CLASS(class);

#define get_object_instance(obj, type) \
    ((type *)(obj))
//    (type *)((char*)(obj) - XtOffsetOf(type, std))

#define get_instance(pz, type) \
    get_object_instance(Z_OBJ_P(pz), type)

#define get_this_instance(type) \
    get_instance(getThis(), type)

#define ION_OBJ(p) &(p)->std

#define ION_OBJ_HANDLERS(p) (p)->std.handlers

#define Z_ISTRUE(zval)				(Z_TYPE(zval) == IS_TRUE)
#define Z_ISTRUE_P(zval_p)			Z_ISTRUE(*(zval_p))

#define Z_ISFALSE(zval)				(Z_TYPE(zval) == IS_FALSE)
#define Z_ISFALSE_P(zval_p)			Z_ISFALSE(*(zval_p))

#define Z_BVAL_P(z) (zend_bool)Z_LVAL_P(z)

#define getThisInstance()             zend_object_store_get_object(this_ptr TSRMLS_CC)
#define getThisInstanceEx(obj_type)   (obj_type) zend_object_store_get_object(this_ptr TSRMLS_CC)

#define PARSE_ARGS(format, ...)                                                 \
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, ##__VA_ARGS__) == FAILURE) {    \
        return;                                                                 \
    }

#define CLASS_METHODS(class_name) methods_ ## class_name

#define CLASS_METHODS_START(class_name) \
    static const zend_function_entry methods_ ## class_name[] = {

#define CLASS_METHODS_END \
        {NULL, NULL, NULL} \
    }

#define RETURN_THIS()   \
    RETVAL_OBJ(Z_OBJ_P(getThis()));            \
    Z_ADDREF_P(return_value); \
    return;

#define zend_object_addref(obj) GC_REFCOUNT(obj)++

#define OBJ_ADDREF(obj) \
    GC_REFCOUNT(obj)++;

#define obj_add_ref(obj) OBJ_ADDREF(obj);

#define OBJ_DELREF(obj) GC_REFCOUNT(obj)--;

#define obj_ptr_dtor(obj) zend_object_release(obj)

#define RETURN_OBJ_ADDREF(obj) \
    OBJ_ADDREF(obj); \
    RETURN_OBJ(obj);

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

#define MAKE_COPY_ZVAL(copy, orig) \
    copy = emalloc(zval);

#define emalloc_instance(type) ecalloc(1, sizeof(type) + zend_object_properties_size(ce))

#define RETURN_INSTANCE(class, object) \
    zend_object_std_init(&object->std, ce); \
    object_properties_init(&object->std, ce); \
    object->std.handlers = &ion_oh_ ## class; \
    return &object->std;

#define pion_register_class(class, class_name, init, methods) \
    pion_register_std_class(&ion_ce_ ## class, class_name, init, methods)

#define pion_register_extended_class(class, parent_class, class_name, init, methods) \
    pion_register_sub_class(&ion_ce_ ## class, parent_class, class_name, init, methods)

#define pion_init_std_object_handlers(class) \
     memcpy(&ion_oh_ ## class, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

#define pion_set_object_handler(class, handler, funct)  \
    ion_oh_ ## class.handler = funct;

#define PION_REGISTER_CLASS(class, class_name)     \
    pion_register_std_class(&ion_ce_ ## class, class_name, _ ## class ## _init, methods_ ## class TSRMLS_CC);   \
    pion_init_std_object_handlers(class); \
    pion_set_object_handler(class, free_obj, _ ## class ## _free);

#define PION_REGISTER_STATIC_CLASS(class, class_name)  PION_REGISTER_DEFAULT_CLASS(class, class_name)

#define PION_REGISTER_DEFAULT_CLASS(class, class_name)                                       \
    pion_register_std_class(&ion_ce_ ## class, class_name, NULL, methods_  ## class);       \
    memcpy(&ion_oh_ ## class, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

#define PION_REGISTER_EXTENDED_CLASS(class, parent_class, class_name) \
    pion_register_sub_class(&c ## class, c ## parent_class, class_name, _ ## class ## _init, methods_ ## class); \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers)); \
    h ## class.free_obj = _ ## class ## _free;

#define PION_REGISTER_EXTENDED_CLASS_EX(class, parent_class, class_name) \
    pion_register_sub_class(&c ## class, c ## parent_class, class_name, _ ## parent_class ## _init, methods_ ## class TSRMLS_CC); \
    pion_init_std_object_handlers(class); \
    pion_set_object_handler(class, free_obj, _ ## class ## _free);

#define REGISTER_EXTENDED_CLASS(class, parent_class, class_name) \
    spl_register_sub_class(&c ## class, ion_get_class(parent_class), class_name, NULL, m ## class TSRMLS_CC); \
    memcpy(&h ## class, zend_get_std_object_handlers(), sizeof (zend_object_handlers));

#define PION_REGISTER_VOID_EXTENDED_CLASS(class, parent_class, class_name) \
    spl_register_sub_class(&ion_ce_ ## class, parent_class, class_name, NULL, NULL); \
    memcpy(&ion_oh_ ## class, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

#define PION_CLASS_CONST_ZVAL(class, const_name, value) \
    zend_declare_class_constant(ion_ce_ ## class, const_name, sizeof(const_name)-1, value);

#define PION_CLASS_CONST_LONG(class, const_name, value) \
    zend_declare_class_constant_long(ion_ce_ ## class, const_name, sizeof(const_name)-1, (long)value);

#define PION_CLASS_CONST_STRING(class, const_name, value) \
    zend_declare_class_constant_string(ion_ce_ ## class, const_name, sizeof(const_name)-1, value);

#define PION_CLASS_PROP_LONG(class, prop_name, value, access) \
    zend_declare_property_long(ion_ce_ ## class, prop_name, sizeof(prop_name)-1, (long)value, access);

#define PION_CLASS_PROP_STRING(class, prop_name, value, access) \
    zend_declare_property_string(ion_ce_ ## class, prop_name, sizeof(prop_name)-1, value, access);

#define PION_CLASS_PROP_BOOL(class, prop_name, value, access) \
    zend_declare_property_bool(ion_ce_ ## class, prop_name, sizeof(prop_name)-1, value, access);

#define pion_update_property_long(class, object_ptr, prop_name, value) \
    zend_update_property_long(ion_ce_ ## class, object_ptr, prop_name, sizeof(prop_name) - 1, value);

#define pion_update_property_str(class, object_ptr, prop_name, value) \
    zend_update_property_str(ion_ce_ ## class, object_ptr, prop_name, sizeof(prop_name) - 1, value);

#define pion_update_property_string(class, object_ptr, prop_name, value) \
    zend_update_property_string(ion_ce_ ## class, object_ptr, prop_name, sizeof(prop_name) - 1, value);

#define pion_update_property_double(class, object_ptr, prop_name, value) \
    zend_update_property_double(ion_ce_ ## class, object_ptr, prop_name, sizeof(prop_name) - 1, value);

#define pion_update_property_bool(class, object_ptr, prop_name, value) \
    zend_update_property_bool(ion_ce_ ## class, object_ptr, prop_name, sizeof(prop_name) - 1, value);


#define CLASS_METHOD(class, method) \
    PHP_METHOD(class, method)

#define METHOD_WITHOUT_ARGS(class_name, method_name) \
    METHOD_ARGS_BEGIN(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()

#define METHOD_WITHOUT_ARGS_RETURN_INT(class_name, method_name) \
    METHOD_ARGS_BEGIN_RETURN_INT(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()

#define METHOD_WITHOUT_ARGS_RETURN_BOOL(class_name, method_name) \
    METHOD_ARGS_BEGIN_RETURN_BOOL(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()

#define METHOD_WITHOUT_ARGS_RETURN_STRING(class_name, method_name) \
    METHOD_ARGS_BEGIN_RETURN_STRING(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()


#define METHOD_WITHOUT_ARGS_RETURN_ARRAY(class_name, method_name) \
    METHOD_ARGS_BEGIN_RETURN_ARRAY(class_name, method_name, 0)    \
    ZEND_END_ARG_INFO()

#define METHOD_ARGS_BEGIN(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args)

#define METHOD_ARGS_BEGIN_RETURN_INT(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_LONG, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_CALLABLE(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_CALLABLE, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_STRING(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_STRING, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_BOOL(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, _IS_BOOL, NULL, 0)

#define METHOD_ARGS_BEGIN_RETURN_OBJECT(class_name, method_name, required_num_args, return_class_name, allow_null) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_OBJECT, return_class_name, allow_null)

#define METHOD_ARGS_BEGIN_RETURN_ARRAY(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_ARRAY, NULL, 0)

#define PION_ZPP_THROW return;

#define _ARGUMENT_HINT_MASK  0x1F
#define ARG_IS_REF           0x20
#define ARG_ALLOW_NULL       0x40
#define ARG_IS_VARIADIC      0x80

#define RET_REF              ARG_IS_REF
#define RET_NULL             ARG_ALLOW_NULL

#define ARGUMENT(name, flags)  { #name, \
                                 NULL, \
                                 flags & _ARGUMENT_HINT_MASK, \
                                 (flags & ARG_IS_REF) ? 1 : 0, \
                                 (flags & ARG_ALLOW_NULL) ? 1 : 0, \
                                 (flags & ARG_IS_VARIADIC) ? 1 : 0 \
                               },

#define METHOD_ARG(name, pass_by_ref)             ZEND_ARG_INFO(pass_by_ref, name)
#define METHOD_ARG_LONG(name, pass_by_ref)        ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_LONG, 0)
#define METHOD_ARG_STRING(name, pass_by_ref)      ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_STRING, 0)
#define METHOD_ARG_DOUBLE(name, pass_by_ref)      ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_DOUBLE, 0)
#define METHOD_ARG_FLOAT(name, pass_by_ref)       METHOD_ARG_DOUBLE(name, pass_by_ref)
#define METHOD_ARG_BOOL(name, pass_by_ref)        ZEND_ARG_TYPE_INFO(pass_by_ref, name, _IS_BOOL, 0)
#define METHOD_ARG_ARRAY(name, pass_by_ref, allow_null)       ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_ARRAY, allow_null)
#define METHOD_ARG_CALLBACK(name, pass_by_ref, allow_null)            ZEND_ARG_TYPE_INFO(pass_by_ref, name, IS_CALLABLE, allow_null)
#define METHOD_ARG_TYPE(name, type_hint, allow_null, pass_by_ref)     ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)
#define METHOD_ARG_OBJECT(name, classname, allow_null, pass_by_ref)   ZEND_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null)

#define METHOD_WITHOUT_ARGS_RETURN(class_name, method_name, flags) \
        ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX( \
        	args ## class_name ## method_name,   \
            (flags & RET_REF) ? 1 : 0,		 	 \
            0, 									 \
            flags & _ARGUMENT_HINT_MASK, 		 \
            NULL, 								 \
            (flags & RET_NULL) ? 1 : 0			 \
        )								   	     \
        ZEND_END_ARG_INFO()                      \


#define METHOD_ARGS_END() \
    ZEND_END_ARG_INFO()

#define ARG_TYPE(name, type_hint, allow_null) \
    ZEND_ARG_TYPE_INFO(0, name, type_hint, allow_null)

#define METHOD(class_name, method_name, flags) \
    ZEND_ME(class_name, method_name, args ## class_name ## method_name, flags)


#define  ZEND_PARSE_PARAMETERS_END_THROW(ION_InvalidArgumentException)  \
            ZEND_PARSE_PARAMETERS_END(pion_throw_invalid_argument_exception(""))

#define PION_INI_BEGIN(module)		static const zend_ini_entry ini_entries[] = {
#define PION_INI_END()		{ 0, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0, 0, NULL } };


PHPAPI void pion_register_interface(zend_class_entry ** ppce, char * class_name, const zend_function_entry * functions);
PHPAPI void pion_register_std_class(zend_class_entry ** ppce, char * class_name, void * obj_ctor, const zend_function_entry * function_list);
PHPAPI void pion_register_sub_class(zend_class_entry ** ppce, zend_class_entry * parent_ce, char * class_name, void *obj_ctor, const zend_function_entry * function_list);

#endif //PION_ENGINE_H
