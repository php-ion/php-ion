#include "strings.h"
#include "init.h"

void ion_interned_strings_ctor(void) {
    zend_string * str = NULL;
#define XX(name, string)                                    \
    str = zend_string_init(string, sizeof(string) - 1, 1);  \
    GC_REFCOUNT(str) = 1;                                   \
    GC_FLAGS(str) |= IS_STR_INTERNED;                       \
    zend_string_hash_val(str);                              \
    GION(interned_strings)[ION_STR_##name] = str;           \

    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
}

void ion_interned_strings_dtor(void) {
#define XX(name, string) pefree((GION(interned_strings)[ION_STR_##name]), 1);
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
}