#include "strings.h"
#include "init.h"

void ion_interned_strings_ctor(void) {
    zend_string * str = NULL;
#define XX(num, name, string)                               \
    str = zend_string_init(string, sizeof(string) - 1, 1);  \
    GC_REFCOUNT(str) = 1;                                   \
    GC_FLAGS(str) |= IS_STR_INTERNED;                       \
    GION(interned_strings)[num] = str;                      \

    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
}

void ion_interned_strings_dtor(void) {
#define XX(num, name, string) zend_string_free(GION(interned_strings)[num]);
    ION_INTERNED_STRINGS_MAP(XX)
#undef XX
}