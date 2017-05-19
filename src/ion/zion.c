
#include "init.h"
#include "zion.h"

ION_API zend_object * ion_init_object(zend_object * php_object, zend_class_entry * ce, zend_object_handlers * php_object_h)
{
    zend_object_std_init(php_object, ce);
    object_properties_init(php_object, ce);
    php_object->handlers = php_object_h;
    
    return php_object;
}