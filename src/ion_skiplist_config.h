#ifndef ION_SKIPLIST_CONFIG_H
#define ION_SKIPLIST_CONFIG_H

#include <php.h>

#define SKIPLIST_MALLOC(sz) emalloc(sz)
#define SKIPLIST_REALLOC(p, sz) erealloc(p, sz)
/* Note - has correct size when freeing, though free does not use it. */
#define SKIPLIST_FREE(p, sz) efree(p)

#endif //ION_SKIPLIST_CONFIG_H
