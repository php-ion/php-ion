#include "Deferred.h"

DEFINE_CLASS(ION_Deferred);

CLASS_INSTANCE_DTOR(ION_Deferred) {
    IONDeferred *defer = getInstanceObject(IONDeferred *);
    efree(defer);
}

CLASS_INSTANCE_CTOR(ION_Deferred) {
    IONDeferred *object = emalloc(sizeof(IONDeferred));
    memset(object, 0, sizeof(IONDeferred));

    RETURN_INSTANCE(ION_Deferred, object);
}

CLASS_METHOD(ION_Deferred, then) {
    IONDeferred *deferred = getThisInstance(IONDeferred *);

    RETURN_NULL();
}

METHOD_ARGS_BEGIN(ION_Deferred, then, 1)
    ARG_TYPE(callback, IS_CALLABLE, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Deferred)
    METHOD(ION_Deferred, then, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_Deferred) {
    PION_REGISTER_CLASS(ION_Deferred, "ION\\Deferred");
    CE(ION_Deferred)->ce_flags |= ZEND_ACC_FINAL_CLASS;

//    REGISTER_VOID_EXTENDED_CLASS(CancelException, Exception, "ION\\CancelException", NULL);
//    REGISTER_VOID_EXTENDED_CLASS(TimeoutException, CancelException, "ION\\TimeoutException", NULL);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Deferred) {
    return SUCCESS;
}