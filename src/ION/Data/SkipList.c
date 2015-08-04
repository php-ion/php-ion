#include "SkipList.h"
#include <ext/spl/spl_iterators.h>
#include <ext/spl/spl_functions.h>


DEFINE_CLASS(ION_Data_SkipList);

#ifdef ZTS
TSRMLS_D ## _local;
#define SYNC_TSRMLS() TSRMLS_D ## _local = TSRMLS_C;
#else
#define SYNC_TSRMLS();
#endif

static int php_skiplist_cmp(void *keyA, void *keyB) {
    zval result;
    TSRMLS_FETCH();
    compare_function(&result, (zval *)keyA, (zval *)keyB TSRMLS_CC);
    return (int)Z_LVAL(result);
}

static void php_skiplist_free(void *key, void *value, void *udata) {
    zval_ptr_dtor((zval **)&key);
    zval_ptr_dtor((zval **)&value);
}

static int php_skiplist_to_array(void *key, void *value, void *udata) {
    add_next_index_zval((zval *)udata, (zval *)value);
    return 0;
}

static int php_skiplist_range(void *key, void *value, void *udata) {
    zval result;
    IONSkipListRange *range = (IONSkipListRange *)udata;
    TSRMLS_FETCH();
    if(range->to) {
        compare_function(&result, (zval *)key, range->to TSRMLS_CC);
        if(Z_LVAL(result) > 0) {
            return 1;
        }
    }
    Z_ADDREF_P((zval *)value);
    add_next_index_zval(range->result, (zval *)value);
    return 0;
}

static IONSkipListRange *get_range(int flags, zval *result, zval *to) {
    IONSkipListRange *range = emalloc(sizeof(IONSkipListRange));
    memset(range, 0, sizeof(IONSkipListRange));
    range->flags = flags;
    range->result = result;
    range->to = to;
    return range;
}

/**
 * Object creating
 */
CLASS_INSTANCE_DTOR(ION_Data_SkipList) {
    IONSkipList *slist = getInstanceObject(IONSkipList *);
    skiplist_free(slist->list, NULL, php_skiplist_free);
    efree(slist);
}

CLASS_INSTANCE_CTOR(ION_Data_SkipList) {
    IONSkipList *llist = emalloc(sizeof(IONSkipList));
    memset(llist, 0, sizeof(IONSkipList));
    llist->list = skiplist_new(php_skiplist_cmp);
    llist->count = 0;

    RETURN_INSTANCE(ION_Data_SkipList, llist);
}

/**
 * Class methods
 */

typedef int (get_from_end_cb)(struct skiplist *list, void **key, void **value);

void get_from_end(struct skiplist *list, zval *return_value, get_from_end_cb cb, int pop) {
    zval *key = NULL;
    zval *val = NULL;
    if(cb(list, (void **)&key, (void **)&val)) {
        RETURN_NULL();
    }
    array_init(return_value);
    if(!pop) {
        Z_ADDREF_P(key);
        Z_ADDREF_P(val);
    }
    add_next_index_zval(return_value, key);
    add_next_index_zval(return_value, val);
}

// PHP API
/** public function ION\Data\SkipList::first() : mixed */
CLASS_METHOD(ION_Data_SkipList, first) {
    get_from_end((getThisInstance(IONSkipList *))->list, return_value, skiplist_first, 0);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, first);

/** public function ION\Data\SkipList::last() : mixed */
CLASS_METHOD(ION_Data_SkipList, last) {
    get_from_end((getThisInstance(IONSkipList *))->list, return_value, skiplist_last, 0);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, last);

/** public function ION\Data\SkipList::rPop() : mixed */
CLASS_METHOD(ION_Data_SkipList, rPop) {
    get_from_end((getThisInstance(IONSkipList *))->list, return_value, skiplist_pop_last, 1);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, rPop);

/** public function ION\Data\SkipList::lPop() : mixed */
CLASS_METHOD(ION_Data_SkipList, lPop) {
    get_from_end((getThisInstance(IONSkipList *))->list, return_value, skiplist_pop_first, 1);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, lPop);

/** public function ION\Data\SkipList::set(key, value) : self */
CLASS_METHOD(ION_Data_SkipList, set) {
    zval *zkey = NULL;
    zval *zvalue = NULL;
    zval *zold = NULL;
    IONSkipList *slist = getThisInstance(IONSkipList *);
    PARSE_ARGS("zz", &zkey, &zvalue);
    Z_ADDREF_P(zkey);
    Z_ADDREF_P(zvalue);
    skiplist_set(slist->list, (void *)zkey, (void *)zvalue, (void **)&zold);
    if(zold) {
        zval_ptr_dtor(&zold);
    }
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, set, 2)
    METHOD_ARG(key, IS_NOT_REF)
    METHOD_ARG(value, IS_NOT_REF)
METHOD_ARGS_END();

/** public function ION\Data\SkipList::add(key, value) : self */
CLASS_METHOD(ION_Data_SkipList, add) {
    zval *zkey = NULL;
    zval *zvalue = NULL;
    IONSkipList *slist = getThisInstance(IONSkipList *);
    PARSE_ARGS("zz", &zkey, &zvalue);
    Z_ADDREF_P(zkey);
    Z_ADDREF_P(zvalue);
    skiplist_add(slist->list, (void *)zkey, (void *)zvalue);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, add, 2)
    METHOD_ARG(key, IS_NOT_REF)
    METHOD_ARG(value, IS_NOT_REF)
METHOD_ARGS_END();


/* public function ION\Data\SkipList::exists(mixed $key) : bool */
CLASS_METHOD(ION_Data_SkipList, exists) {
    zval *key = NULL;
    int result = 0;
    PARSE_ARGS("z", &key);
    result = skiplist_member((getThisInstance(IONSkipList *))->list, (void *)key);
    if(result) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, exists, 1)
    METHOD_ARG(key, IS_NOT_REF)
METHOD_ARGS_END();


/* public function ION\Data\SkipList::get(mixed $key, bool $all = false) : mixed */
CLASS_METHOD(ION_Data_SkipList, get) {
    zval *key = NULL;
    zend_bool *all = 0;
    PARSE_ARGS("z|b", &key, &all);
    if(all) { // get all values by key
        array_init(return_value);
        IONSkipListRange *range = get_range(ION_SKIPLIST_RANGE_WITHOUT_KEYS, return_value, key);
        skiplist_iter_from(
                (getThisInstance(IONSkipList *))->list,
                (void *) key,
                (void *) range,
                php_skiplist_range
        );
        efree(range);
    } else { // get any single value by key
        zval *value = (zval *)skiplist_get((getThisInstance(IONSkipList *))->list, (void *)key);
        if(value == NULL) {
            RETURN_NULL();
        }
        RETVAL_ZVAL(value, 1, 0);
    }
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, get, 1)
    METHOD_ARG(key, IS_NOT_REF)
    METHOD_ARG(all, IS_NOT_REF)
METHOD_ARGS_END();


/* public function ION\Data\SkipList::count() : int */
CLASS_METHOD(ION_Data_SkipList, count) {
    IONSkipList *slist = getThisInstance(IONSkipList *);
    RETURN_LONG(skiplist_count(slist->list));
}

METHOD_WITHOUT_ARGS(ION_Data_SkipList, count);


/* public function ION\Data\SkipList::toArray() : array */
CLASS_METHOD(ION_Data_SkipList, toArray) {
    array_init(return_value);
    skiplist_iter(
        (getThisInstance(IONSkipList *))->list,
        (void *) return_value,
        php_skiplist_to_array
    );
}

METHOD_WITHOUT_ARGS(ION_Data_SkipList, toArray);

CLASS_METHODS_START(ION_Data_SkipList)
    METHOD(ION_Data_SkipList, first, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, last, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, rPop, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, lPop, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, set, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, add, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, exists, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, get, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, count, ZEND_ACC_PUBLIC)
    METHOD(ION_Data_SkipList, toArray, ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Data_SkipList) {
    SKIPLIST_GEN_HEIGHT();

    PION_REGISTER_CLASS(ION_Data_SkipList, "ION\\Data\\SkipList");
    zend_class_implements(CE(ION_Data_SkipList) TSRMLS_CC, 1, spl_ce_Countable);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Data_SkipList) {
    return SUCCESS;
}
