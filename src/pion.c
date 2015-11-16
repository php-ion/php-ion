
#include "php.h"
#include "pion.h"

BEGIN_EXTERN_C()

ZEND_API void * php_emalloc_wrapper(size_t size) {
    if(size) {
        return emalloc(size);
    } else {
        return NULL;
    }
}

ZEND_API void * php_realloc_wrapper(void * nmemb, size_t size) {
    if(size) {
        if(!nmemb) {
            return emalloc(size);
        } else {
            return erealloc(nmemb, size);
        }
    } else if(nmemb) {
        efree(nmemb);
    }
    return NULL;
}

ZEND_API void php_efree_wrapper(void * ptr) {
    if(ptr) {
        efree(ptr);
    }
}


ZEND_API zend_bool ion_reinit() {
    zend_bool result;
    event_set_mem_functions(malloc, realloc, free);
    if(event_reinit(ION(base)) == FAILURE) {
        result = false;
    } else {
        result = true;
    }
    event_set_mem_functions(php_emalloc_wrapper, php_realloc_wrapper, php_efree_wrapper);

    return result;
}


END_EXTERN_C()