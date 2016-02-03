

#include "engine.h"

PHPAPI void pion_register_interface(zend_class_entry ** ppce, char * class_name, const zend_function_entry * functions)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY_EX(ce, class_name, strlen(class_name), functions);
    *ppce = zend_register_internal_interface(&ce);
}

PHPAPI void pion_register_std_class(zend_class_entry ** ppce, char * class_name, void * obj_ctor, const zend_function_entry * function_list)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY_EX(ce, class_name, strlen(class_name), function_list);
    *ppce = zend_register_internal_class(&ce);

    /* entries changed by initialize */
    if (obj_ctor) {
        (*ppce)->create_object = obj_ctor;
    }
}

PHPAPI void pion_register_sub_class(zend_class_entry ** ppce, zend_class_entry * parent_ce, char * class_name, void *obj_ctor, const zend_function_entry * function_list)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY_EX(ce, class_name, strlen(class_name), function_list);
    *ppce = zend_register_internal_class_ex(&ce, parent_ce);

    /* entries changed by initialize */
    if (obj_ctor) {
        (*ppce)->create_object = obj_ctor;
    } else {
        (*ppce)->create_object = parent_ce->create_object;
    }
}
