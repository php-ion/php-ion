/**
 * Zend ION integration functions
 */
#ifndef ION_ZION_H
#define ION_ZION_H

#ifndef ION_API
#define ION_API
#endif

/** Classes **/
// Init
//#define ion_class_set_offset(handler, struct_object) handler.offset  = (int)XtOffsetOf(struct_object, php_object);
#define ion_offset(struct_object) (int)XtOffsetOf(struct_object, php_object)
#define ion_init_object_handlers(handlers)  memcpy(&handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

ION_API void ion_register_interface(zend_class_entry ** ppce, char * class_name, const zend_function_entry * functions);
ION_API void ion_register_class_ex(zend_class_entry ** ppce, zend_class_entry * parent_ce, char * class_name, void *obj_ctor, const zend_function_entry * function_list);
#define ion_register_class(pce, class_name, ctor, methods) \
        ion_register_class_ex(&pce, NULL, class_name, ctor, methods)
#define ion_register_exception(pce, parent_ce, class_name) \
        ion_register_class_ex(&pce, parent_ce, class_name, NULL, NULL)
#define ion_register_static_class(pce, class_name, methods) \
        ion_register_class_ex(&pce, NULL, class_name, NULL, methods)

// Properties
#define ion_class_declare_property_null(ce, prop_name, access) \
    zend_declare_property_null(ce, prop_name, sizeof(prop_name)-1, access)
#define ion_class_declare_property_long(ce, prop_name, value, access) \
    zend_declare_property_long(ce, prop_name, sizeof(prop_name)-1, (long)value, access)
#define ion_class_declare_property_string(ce, prop_name, value, access) \
    zend_declare_property_string(ce, prop_name, sizeof(prop_name)-1, value, access)
#define ion_class_declare_property_bool(ce, prop_name, value, access) \
    zend_declare_property_bool(ce, prop_name, sizeof(prop_name)-1, (int)value, access)

#define ion_update_property_long(ce, object_ptr, prop_name, value) \
    zend_update_property_long(ce, object_ptr, prop_name, sizeof(prop_name) - 1, value);
#define ion_update_property_str(ce, object_ptr, prop_name, value) \
    zend_update_property_str(ce, object_ptr, prop_name, sizeof(prop_name) - 1, value);
#define ion_update_property_string(ce, object_ptr, prop_name, value) \
    zend_update_property_string(ce, object_ptr, prop_name, sizeof(prop_name) - 1, value);
#define ion_update_property_double(ce, object_ptr, prop_name, value) \
    zend_update_property_double(ce, object_ptr, prop_name, sizeof(prop_name) - 1, value);
#define ion_update_property_bool(ce, object_ptr, prop_name, value) \
    zend_update_property_bool(ce, object_ptr, prop_name, sizeof(prop_name) - 1, value);



// Constants
#define ion_class_declare_constant_zval(ce, const_name, value) \
    zend_declare_class_constant(ce, const_name, sizeof(const_name)-1, value)
#define ion_class_declare_constant_bool(ce, const_name, value) \
    zend_declare_class_constant_bool(ce, const_name, sizeof(const_name)-1, value)
#define ion_class_declare_constant_long(ce, const_name, value) \
    zend_declare_class_constant_long(ce, const_name, sizeof(const_name)-1, (long)value)
#define ion_class_declare_constant_string(ce, const_name, value) \
    zend_declare_class_constant_string(ce, const_name, sizeof(const_name)-1, value)
/** End Classes  **/

/** Functions and methods **/

#define CLASS_METHOD(class, method) \
    PHP_METHOD(class, method)

#define METHOD(class_name, method_name, flags) \
    ZEND_ME(class_name, method_name, args ## class_name ## method_name, flags)

#define METHODS_START(methods) \
    static const zend_function_entry methods[] = {

#define METHODS_END        \
        {NULL, NULL, NULL} \
    }

#define _ARGUMENT_HINT_MASK  0x1F
#define ARG_IS_REF           0x20
#define ARG_ALLOW_NULL       0x40
#define ARG_IS_VARIADIC      0x80

#define IS_BOOLEAN _IS_BOOL
#define IS_MIXED   0

#define RET_REF              ARG_IS_REF
#define RET_NULL             ARG_ALLOW_NULL


#ifdef IS_PHP71

# define ARGUMENT(name, flags)  {                   \
      #name,                                        \
      NULL,                                         \
      (flags) & _ARGUMENT_HINT_MASK,                \
      ((flags) & ARG_IS_REF) ? 1 : 0,               \
      ((flags) & ARG_ALLOW_NULL) ? 1 : 0,           \
      ((flags) & ARG_IS_VARIADIC) ? 1 : 0           \
      (((flags) & ARG_IS_VARIADIC) ? 1 : 0)         \
    },

# define ARGUMENT_OBJECT(name, classname, flags)  { \
      #name,                                        \
      #classname,                                   \
      IS_OBJECT,                                    \
      ((flags) & ARG_IS_REF) ? 1 : 0,               \
      ((flags) & ARG_ALLOW_NULL) ? 1 : 0,           \
      ((flags) & ARG_IS_VARIADIC) ? 1 : 0           \
    },

 #define METHOD_WITHOUT_ARGS_RETURN(class_name, method_name, flags) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX( \
        args ## class_name ## method_name,   \
        (flags & RET_REF) ? 1 : 0,		 	 \
        0, 								     \
        flags & _ARGUMENT_HINT_MASK, 		 \
        NULL, 								 \
        (flags & RET_NULL) ? 1 : 0			 \
    )								   	     \
    ZEND_END_ARG_INFO()                      \

#else

# define ARGUMENT(name, flags)  {                   \
        #name,                                      \
        ZEND_TYPE_ENCODE(                           \
         (flags) & _ARGUMENT_HINT_MASK,             \
         (((flags) & ARG_ALLOW_NULL) ? 1 : 0)       \
        ),                                          \
        (((flags) & ARG_IS_REF) ? 1 : 0),           \
        (((flags) & ARG_IS_VARIADIC) ? 1 : 0)       \
    },

# define ARGUMENT_OBJECT(name, classname, flags)  { \
     #name,                                        \
     ZEND_TYPE_ENCODE_CLASS_CONST(                 \
        #classname,                                \
        0                                          \
     ),                                            \
     (((flags) & ARG_IS_REF) ? 1 : 0),             \
     (((flags) & ARG_IS_VARIADIC) ? 1 : 0)         \
   },

# define METHOD_WITHOUT_ARGS_RETURN(class_name, method_name, flags) \
        ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX( \
        	args ## class_name ## method_name,   \
            (((flags) & RET_REF) ? 1 : 0),		 \
            0, 									 \
            ((flags) & _ARGUMENT_HINT_MASK), 	 \
            (((flags) & RET_NULL) ? 1 : 0)	     \
        )								   	     \
        ZEND_END_ARG_INFO()                      \

#endif

#define METHOD_ARGS_END() \
    ZEND_END_ARG_INFO()


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
#define METHOD_ARGS_BEGIN_RETURN_VOID(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_INFO_EX(args ## class_name ## method_name, 0, 0, required_num_args)


#ifdef IS_PHP71
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
#else
#define METHOD_ARGS_BEGIN_RETURN_INT(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_LONG, 0)
#define METHOD_ARGS_BEGIN_RETURN_CALLABLE(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_CALLABLE, 0)
#define METHOD_ARGS_BEGIN_RETURN_STRING(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_STRING, 0)
#define METHOD_ARGS_BEGIN_RETURN_BOOL(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, _IS_BOOL, 0)
#define METHOD_ARGS_BEGIN_RETURN_OBJECT(class_name, method_name, required_num_args, return_class_name, allow_null) \
    ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(args ## class_name ## method_name, 0, required_num_args, return_class_name, allow_null)
#define METHOD_ARGS_BEGIN_RETURN_ARRAY(class_name, method_name, required_num_args) \
    ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(args ## class_name ## method_name, 0, required_num_args, IS_ARRAY, 0)
#endif



/** End functions and methods **/

/** Arguments **/

#define PION_ZPP_THROW return;

/** End arguments **/

/** Objects  **/
#define ion_alloc_object(ce, struct_object) ecalloc(1, sizeof(struct_object) + zend_object_properties_size(ce))
ION_API zend_object * ion_init_object(zend_object * php_object, zend_class_entry * ce, zend_object_handlers * php_object_h);
#define ION_ZOBJ_OBJECT(zobj, struct_object) ((struct_object *)((char *)zobj - XtOffsetOf(struct_object, php_object)))
#define ION_OBJECT_ZOBJ(object) (&(object->php_object))
#define ION_ZVAL_OBJECT_P(pzv, struct_object) ION_ZOBJ_OBJECT(Z_OBJ_P(pzv), struct_object)
#define ION_ZVAL_OBJECT(zv, struct_object) ION_ZOBJ_OBJECT(Z_OBJ(zv), struct_object)
#define ION_THIS_OBJECT(struct_object) ION_ZVAL_OBJECT_P(getThis(), struct_object)
#define RETURN_ION_OBJ(object) RETURN_OBJ(ION_OBJECT_ZOBJ(object))

// same as zend_object_release() but only for ion objects
#define ion_object_release(obj) zend_object_release(ION_OBJECT_ZOBJ(obj));
#define ion_object_addref(obj) zend_object_addref(ION_OBJECT_ZOBJ(obj));

#define RETURN_THIS()   \
    RETVAL_OBJ(Z_OBJ_P(getThis()));            \
    Z_ADDREF_P(return_value); \
    return;

#define zend_object_addref(obj) GC_REFCOUNT(obj)++

/** End Objects **/


/** Zval **/

#define Z_ISTRUE(zval)				(Z_TYPE(zval) == IS_TRUE)
#define Z_ISTRUE_P(zval_p)			Z_ISTRUE(*(zval_p))

#define Z_ISFALSE(zval)				(Z_TYPE(zval) == IS_FALSE)
#define Z_ISFALSE_P(zval_p)			Z_ISFALSE(*(zval_p))

/** End Zval **/
#endif //ION_ZION_H
