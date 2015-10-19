#ifndef ION_SKIPLIST_H
#define ION_SKIPLIST_H

#include <php.h>
#include <zend.h>
#include "../../pion.h"
#include "../../externals/SkipList/skiplist.h"

// LinkedList PHP object
typedef struct _ion_skiplist {
    zend_object       std;
    struct skiplist  *list;
    long              count;
} ion_skiplist;

typedef struct _ion_skiplist_range {
    int      flags;
    zval    *result;
    zval    *prev_key;
    zval    *to;
#ifdef ZTS
    void ***thread_ctx;
#endif
} ion_skiplist_range;

static ion_skiplist_range * _ion_skiplist_get_range(int flags, zval * result, zval * to TSRMLS_DC);

#define  ion_skiplist_get_range(flags, result, to) _ion_skiplist_get_range(flags, result, to TSRMLS_CC)

#define ION_SKIPLIST_RANGE_WITH_KEYS 1
#define ION_SKIPLIST_RANGE_WITHOUT_KEYS  0

#endif //ION_SKIPLIST_H
