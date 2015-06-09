//
// Created by Ivan Shalganov on 08.06.15.
//

#ifndef ION_SKIPLIST_H
#define ION_SKIPLIST_H

#include <php.h>
#include <zend.h>
#include "../../framework.h"
#include "../../SkipList/skiplist.h"

// LinkedList PHP object
typedef struct _php_skiplist {
    zend_object       std;
    struct skiplist  *list;
    long              count;
} IONSkipList;

PHP_MINIT_FUNCTION(ION_Data_SkipList);
DEFINE_CLASS(ION_Data_SkipList);

PHP_METHOD(ION_Data_SkipList, first);
PHP_METHOD(ION_Data_SkipList, last);
PHP_METHOD(ION_Data_SkipList, rPop);
PHP_METHOD(ION_Data_SkipList, lPop);
PHP_METHOD(ION_Data_SkipList, count);
PHP_METHOD(ION_Data_SkipList, exists);
PHP_METHOD(ION_Data_SkipList, get);
PHP_METHOD(ION_Data_SkipList, set);
PHP_METHOD(ION_Data_SkipList, add);

#endif //ION_SKIPLIST_H
