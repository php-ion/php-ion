#include "strings.h"
#include "init.h"

void ion_interned_strings_ctor(void) {
    zend_ion_global_cache * cache = GION(cache);
    zend_string * str = NULL;

#define XX(name, string)                                    \
    str = zend_string_init(string, sizeof(string) - 1, 1);  \
    GC_REFCOUNT(str) = 1;                                   \
    GC_FLAGS(str) |= IS_STR_INTERNED;                       \
    zend_string_hash_val(str);                              \
    cache->interned_strings[ION_STR_##name] = str;          \

    ION_INTERNED_STRINGS_MAP(XX);
#undef XX
}

void ion_interned_strings_dtor(void) {
    zend_ion_global_cache * cache = GION(cache);

#define XX(name, string) pefree((cache->interned_strings[ION_STR_##name]), 1);
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
}

void ion_interned_strings_to_array(zend_array * array, size_t from, size_t to) {
    zend_ion_global_cache * cache = GION(cache);
    zval index;
    if(to > ION_STRINGS_SIZE) {
        to = ION_STRINGS_SIZE;
    }

    for(size_t i = from; i <= to; i++) {
        ZVAL_LONG(&index, i);
        zend_hash_add(array, cache->interned_strings[i], &index);
    }
}