#include "SkipList.h"
#include <ext/spl/spl_iterators.h>
#include <ext/spl/spl_functions.h>


DEFINE_CLASS(ION_Data_SkipList);

static int php_skiplist_cmp(void *keyA, void *keyB) {
    zval result;
    compare_function(&result, (zval *)keyA, (zval *)keyB);
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

    if(range->to) {
//        PHPDBG("compare");
//        ZVAL_DUMP_P((zval *)key);
        compare_function(&result, (zval *)key, range->to);
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
static void _ion_slist_dtor(void *object TSRMLS_DC) {
    IONSkipList *slist = (IONSkipList *) object;
    skiplist_free(slist->list, NULL, php_skiplist_free);
    efree(slist);
}

static zend_object_value _ion_slist_ctor(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    IONSkipList *llist = emalloc(sizeof(IONSkipList));
    memset(llist, 0, sizeof(IONSkipList));
    llist->list = skiplist_new(php_skiplist_cmp);
    llist->count = 0;
    OBJECT_INIT(retval, ION_Data_SkipList, llist, _ion_slist_dtor);
    return retval;
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
METHOD(ION_Data_SkipList, first) {
    get_from_end((this_get_object_ex(IONSkipList *))->list, return_value, skiplist_first, 0);
}

METHOD(ION_Data_SkipList, last) {
    get_from_end((this_get_object_ex(IONSkipList *))->list, return_value, skiplist_last, 0);
}

METHOD(ION_Data_SkipList, rPop) {
    get_from_end((this_get_object_ex(IONSkipList *))->list, return_value, skiplist_pop_last, 1);
}

METHOD(ION_Data_SkipList, lPop) {
    get_from_end((this_get_object_ex(IONSkipList *))->list, return_value, skiplist_pop_first, 1);
}

METHOD(ION_Data_SkipList, set) {
    zval *zkey = NULL;
    zval *zvalue = NULL;
    zval *zold = NULL;
    IONSkipList *slist = this_get_object_ex(IONSkipList *);
    PARSE_ARGS("zz", &zkey, &zvalue);
    Z_ADDREF_P(zkey);
    Z_ADDREF_P(zvalue);
    skiplist_set(slist->list, (void *)zkey, (void *)zvalue, (void **)&zold);
    if(zold) {
        zval_ptr_dtor(&zold);
    }
    RETURN_THIS();
}

METHOD_ARGS_LIST_BEGIN(ION_Data_SkipList, set, 2)
    METHOD_ARG(key, IS_NOT_REF)
    METHOD_ARG(value, IS_NOT_REF)
METHOD_ARGS_LIST_END();

METHOD(ION_Data_SkipList, add) {
    zval *zkey = NULL;
    zval *zvalue = NULL;
    IONSkipList *slist = this_get_object_ex(IONSkipList *);
    PARSE_ARGS("zz", &zkey, &zvalue);
    Z_ADDREF_P(zkey);
    Z_ADDREF_P(zvalue);
    skiplist_add(slist->list, (void *)zkey, (void *)zvalue);
    RETURN_THIS();
}

METHOD_ARGS_LIST_BEGIN(ION_Data_SkipList, add, 2)
    METHOD_ARG(key, IS_NOT_REF)
    METHOD_ARG(value, IS_NOT_REF)
METHOD_ARGS_LIST_END();

/* ION\Data\SkipList::exists(mixed $key) : bool */
METHOD(ION_Data_SkipList, exists) {
    zval *key = NULL;
    int result = 0;
    PARSE_ARGS("z", &key);
    result = skiplist_member((this_get_object_ex(IONSkipList *))->list, (void *)key);
    if(result) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_ARGS_LIST_BEGIN(ION_Data_SkipList, exists, 1)
    METHOD_ARG(key, IS_NOT_REF)
METHOD_ARGS_LIST_END();

/* ION\Data\SkipList::get(mixed $key, bool $all = false) : mixed */
METHOD(ION_Data_SkipList, get) {
    zval *key = NULL;
    zend_bool *all = 0;
    PARSE_ARGS("z|b", &key, &all);
    if(all) { // get all values by key
        array_init(return_value);
        IONSkipListRange *range = get_range(ION_SKIPLIST_RANGE_WITHOUT_KEYS, return_value, key);
        skiplist_iter_from(
                (this_get_object_ex(IONSkipList *))->list,
                (void *) key,
                (void *) range,
                php_skiplist_range
        );
        efree(range);
    } else { // get any single value by key
        zval *value = (zval *)skiplist_get((this_get_object_ex(IONSkipList *))->list, (void *)key);
        if(value == NULL) {
            RETURN_NULL();
        }
        RETVAL_ZVAL(value, 1, 0);
    }
}

METHOD_ARGS_LIST_BEGIN(ION_Data_SkipList, get, 1)
    METHOD_ARG(key, IS_NOT_REF)
    METHOD_ARG(all, IS_NOT_REF)
METHOD_ARGS_LIST_END();


METHOD(ION_Data_SkipList, count) {
    IONSkipList *slist = this_get_object_ex(IONSkipList *);
    RETURN_LONG(skiplist_count(slist->list));
}

/* ION\Data\SkipList::toArray() : array */
METHOD(ION_Data_SkipList, toArray) {
    array_init(return_value);
    skiplist_iter(
            (this_get_object_ex(IONSkipList *))->list,
            (void *) return_value,
            php_skiplist_to_array
    );
}

CLASS_METHODS_START(ION_Data_SkipList)
    METHOD_NOARGS(ION_Data_SkipList, first,  ZEND_ACC_PUBLIC)
    METHOD_NOARGS(ION_Data_SkipList, last,  ZEND_ACC_PUBLIC)
    METHOD_NOARGS(ION_Data_SkipList, rPop,   ZEND_ACC_PUBLIC)
    METHOD_NOARGS(ION_Data_SkipList, lPop,   ZEND_ACC_PUBLIC)
    METHOD_ARGS(ION_Data_SkipList, set,   ZEND_ACC_PUBLIC)
    METHOD_ARGS(ION_Data_SkipList, add,   ZEND_ACC_PUBLIC)
    METHOD_ARGS(ION_Data_SkipList, exists, ZEND_ACC_PUBLIC)
    METHOD_ARGS(ION_Data_SkipList, get,    ZEND_ACC_PUBLIC)
    METHOD_NOARGS(ION_Data_SkipList, count,  ZEND_ACC_PUBLIC)
    METHOD_NOARGS(ION_Data_SkipList, toArray,  ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Data_SkipList) {
    SKIPLIST_GEN_HEIGHT();

    REGISTER_CLASS(ION_Data_SkipList, "ION\\Data\\SkipList", _ion_slist_ctor);
    zend_class_implements(CE(ION_Data_SkipList) TSRMLS_CC, 1, spl_ce_Countable);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Data_SkipList) {
    return SUCCESS;
}
