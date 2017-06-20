
#include "ion_init.h"
#include "ion_zend.h"

ION_API zend_object * ion_init_object(zend_object * php_object, zend_class_entry * ce, zend_object_handlers * php_object_h)
{
    zend_object_std_init(php_object, ce);
    object_properties_init(php_object, ce);
    php_object->handlers = php_object_h;
    
    return php_object;
}


ION_API void ion_register_interface(zend_class_entry ** ppce, char * class_name, const zend_function_entry * functions)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY_EX(ce, class_name, strlen(class_name), functions);
    *ppce = zend_register_internal_interface(&ce);
}

ION_API void ion_register_class_ex(zend_class_entry ** ppce, zend_class_entry * parent_ce, char * class_name, void *obj_ctor, const zend_function_entry * function_list)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY_EX(ce, class_name, strlen(class_name), function_list);
    if (parent_ce) {
        *ppce = zend_register_internal_class_ex(&ce, parent_ce);
    } else {
        *ppce = zend_register_internal_class(&ce);
    }

    /* entries changed by initialize */
    if (obj_ctor) {
        (*ppce)->create_object = obj_ctor;
    } else if(parent_ce) {
        (*ppce)->create_object = parent_ce->create_object;
    }
}
