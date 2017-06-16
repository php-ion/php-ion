/**
 * Zend ION integration functions
 */
#ifndef ION_ZION_H
#define ION_ZION_H

#ifndef ION_API
#define ION_API
#endif

/** Classes **/
#define ion_class_set_offset(handler, struct_object) handler.offset  = (int)XtOffsetOf(struct_object, php_object);

#define ion_class_declare_property_long(ce, prop_name, value, access) \
    zend_declare_property_long(ce, prop_name, sizeof(prop_name)-1, (long)value, access)
#define ion_class_declare_property_string(ce, prop_name, value, access) \
    zend_declare_property_string(ce, prop_name, sizeof(prop_name)-1, value, access)
#define ion_class_declare_property_bool(ce, prop_name, value, access) \
    zend_declare_property_bool(ce, prop_name, sizeof(prop_name)-1, (int)value, access)
/** End Classes  **/

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

//#define
/** End Objects **/

/** Zval **/

#define Z_ISTRUE(zval)				(Z_TYPE(zval) == IS_TRUE)
#define Z_ISTRUE_P(zval_p)			Z_ISTRUE(*(zval_p))

#define Z_ISFALSE(zval)				(Z_TYPE(zval) == IS_FALSE)
#define Z_ISFALSE_P(zval_p)			Z_ISFALSE(*(zval_p))

/** End Zval **/
#endif //ION_ZION_H
