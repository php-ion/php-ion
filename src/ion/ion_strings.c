#include "ion_strings.h"
#include "ion_init.h"

void ion_interned_strings_ctor(void) {
    zend_ion_global_cache * cache = GION(cache);
    zend_string * str = NULL;
    zval          index;

    cache->index = (HashTable *) pemalloc(sizeof(HashTable), 1);
    zend_hash_init(cache->index, ION_STR_LAST, NULL, NULL, 1);
#define XX(name, string)                                    \
    str = zend_string_init(string, sizeof(string) - 1, 1);  \
    GC_SET_REFCOUNT(str, 1);                                \
    GC_ADD_FLAGS(str, IS_STR_INTERNED);                     \
    zend_string_hash_val(str);                              \
    cache->interned_strings[ION_STR_##name] = str;          \
    ZVAL_LONG(&index, ION_STR_##name);                      \
    zend_hash_add(cache->index, str, &index);       \

    ION_INTERNED_STRINGS_MAP(XX);
#undef XX
}

void ion_interned_strings_dtor(void) {
    zend_ion_global_cache * cache = GION(cache);

    zend_hash_destroy(cache->index);
    pefree(cache->index, 1);

#define XX(name, string) pefree((cache->interned_strings[ION_STR_##name]), 1);
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
}
