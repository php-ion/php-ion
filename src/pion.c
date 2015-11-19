
#include "php.h"
#include "pion.h"

BEGIN_EXTERN_C()

// See http://www.wangafu.net/~nickm/libevent-book/Ref1_libsetup.html

/* This union's purpose is to be as big as the largest of all the
 * types it contains. */
union alignment {
    size_t sz;
    void *ptr;
    double dbl;
};
/* We need to make sure that everything we return is on the right
   alignment to hold anything, including a double. */
#define ALIGNMENT sizeof(union alignment)

/* We need to do this cast-to-char* trick on our pointers to adjust
   them; doing arithmetic on a void* is not standard. */
#define OUTPTR(ptr) (((char*)ptr)+ALIGNMENT)
#define INPTR(ptr) (((char*)ptr)-ALIGNMENT)

ZEND_API void * php_emalloc_wrapper(size_t size) {
    void *chunk = malloc(size + ALIGNMENT);
    if (!chunk) return chunk;
    *(size_t*)chunk = size;
    return OUTPTR(chunk);
}

ZEND_API void * php_realloc_wrapper(void * ptr, size_t size) {
    if (ptr) {
        ptr = INPTR(ptr);
    }
    ptr = realloc(ptr, size + ALIGNMENT);
    if (!ptr)
        return NULL;
    *(size_t*)ptr = size;
    return OUTPTR(ptr);
}

ZEND_API void php_efree_wrapper(void * ptr) {
    ptr = INPTR(ptr);
    free(ptr);
}

END_EXTERN_C()