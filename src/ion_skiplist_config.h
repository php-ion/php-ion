#ifndef ION_SKIPLIST_CONFIG_H
#define ION_SKIPLIST_CONFIG_H

#include <php.h>

#define SKIPLIST_MALLOC(sz) emalloc(sz)
#define SKIPLIST_REALLOC(p, sz) erealloc(p, sz)
#define SKIPLIST_FREE(p, sz) efree(p)
#define SKIPLIST_MAX_HEIGHT 16

#endif //ION_SKIPLIST_CONFIG_H
