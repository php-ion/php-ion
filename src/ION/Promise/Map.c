#include "Map.h"
#include "../Promise.h"

ION_DEFINE_CLASS(ION_Promise_Map);


/** public function ION\Promise\Map::__construct(callable $handler) : self */
CLASS_METHOD(ION_Promise_Map, __construct) {
    ion_promise * promise = getThisInstance();
    promise->flags |= ION_PROMISE_MAP;
    zval * handler = NULL;
    PARSE_ARGS("z", &handler);
    ion_promise_set_callbacks(getThis(), handler, NULL, NULL);
}

METHOD_ARGS_BEGIN(ION_Promise_Map, __construct, 1)
    METHOD_ARG_CALLBACK(handler, 0, 0)
METHOD_ARGS_END()


/** public function ION\Promise\Map::__invoke(mixed $data) : self */
CLASS_METHOD(ION_Promise_Map, __invoke) {
    zval * data = NULL;
    zval * clone = NULL;
    zend_object_value * clone_obj = NULL;
    PARSE_ARGS("z", &data);
    clone_obj = zend_objects_store_clone_obj(getThis() TSRMLS_DC);
    ALLOC_INIT_ZVAL(clone);
    Z_TYPE_P(clone) = IS_OBJECT;
    Z_OBJVAL_P()
    ion_promise_done(clone, data);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Promise_Map, __invoke, 1)
   METHOD_ARG(data, 0)
METHOD_ARGS_END()


CLASS_METHODS_START(ION_Promise_Map)
    METHOD(ION_Promise_Map, __construct,       ZEND_ACC_PUBLIC)
    METHOD(ION_Promise_Map, __invoke,          ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Promise_Map) {
    REGISTER_EXTENDED_CLASS(ION_Promise_Map, ION_Promise, "ION\\Promise\\Map");
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_Promise_Map) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_Promise_Map) {
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Promise_Map) {
    return SUCCESS;
}