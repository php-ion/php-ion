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
} IONSkipList;

typedef struct _ion_skiplist_range {
    int      flags;
    zval    *result;
    zval    *prev_key;
    zval    *to;
#ifdef ZTS
    void ***thread_ctx;
#endif
} IONSkipListRange;

static IONSkipListRange *IONSListGetRange(int flags, zval *result, zval *to TSRMLS_DC);

#define ION_SKIPLIST_RANGE_WITH_KEYS 1
#define ION_SKIPLIST_RANGE_WITHOUT_KEYS  0

PHP_MINIT_FUNCTION(ION_Data_SkipList);
DEFINE_CLASS(ION_Data_SkipList);

PHP_METHOD(ION_Data_SkipList, first);
PHP_METHOD(ION_Data_SkipList, last);
PHP_METHOD(ION_Data_SkipList, rPop);
PHP_METHOD(ION_Data_SkipList, lPop);
PHP_METHOD(ION_Data_SkipList, count);
PHP_METHOD(ION_Data_SkipList, exists);
PHP_METHOD(ION_Data_SkipList, get);
PHP_METHOD(ION_Data_SkipList, getAll);
PHP_METHOD(ION_Data_SkipList, set);
PHP_METHOD(ION_Data_SkipList, add);

#endif //ION_SKIPLIST_H
