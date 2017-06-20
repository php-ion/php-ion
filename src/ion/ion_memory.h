#ifndef ION_CORE_MEMORY_H
#define ION_CORE_MEMORY_H

ION_API void * php_emalloc_wrapper(size_t size);
ION_API void * php_realloc_wrapper(void * nmemb, size_t size);
ION_API void   php_efree_wrapper(void * ptr);

#endif //ION_CORE_MEMORY_H
