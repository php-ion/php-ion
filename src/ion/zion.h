/**
 * Zend ION integration functions
 */
#ifndef ION_ZION_H
#define ION_ZION_H

/** Classes **/
#define ion_class_set_offset(handler, struct_object) handler.offset  = (int)XtOffsetOf(struct_object, php_object);
/** End Classes  **/

/** Objects  **/
#define ion_alloc_object(ce, struct_object) ecalloc(1, sizeof(struct_object) + zend_object_properties_size(ce))
ION_API zend_object * ion_init_object(zend_object * php_object, zend_class_entry * ce, zend_object_handlers * php_object_h);
#define ION_ZOBJ_OBJECT(zobj, struct_object) ((struct_object *)((char *)zobj - XtOffsetOf(struct_object, php_object)))
#define ION_OBJECT_ZOBJ(object) (&object->php_object)
#define ION_ZVAL_OBJECT_P(pzv, struct_object) ION_ZOBJ_OBJECT(Z_OBJ_P(pzv), struct_object)
#define ION_ZVAL_OBJECT(zv, struct_object) ION_ZOBJ_OBJECT(Z_OBJ(zv), struct_object)
#define ION_THIS_OBJECT(struct_object) ION_ZVAL_OBJECT_P(getThis(), struct_object)
/** End Objects **/

#endif //ION_ZION_H
