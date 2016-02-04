#include "SkipList.h"
#include <ext/spl/spl_iterators.h>


ION_DEFINE_CLASS(ION_Data_SkipList);

static int php_skiplist_cmp(void *keyA, void *keyB) {
    zval result;
    TSRMLS_FETCH();
    compare_function(&result, (zval *)keyA, (zval *)keyB TSRMLS_CC);
    return (int)Z_LVAL(result);
}

static void php_skiplist_free(void * k, void * v, void * udata) {
    zval * key = (zval *)k;
    zval * value = (zval *)v;
    zval_ptr_dtor(key);
    efree(key);
    zval_ptr_dtor(value);
    efree(value);
}

static int php_skiplist_to_array(void * key, void * value, void * udata) {
    add_next_index_zval((zval *) udata, (zval *) value);
    return 0;
}

static int php_skiplist_range(void * key, void * value, void * udata) {
    zval result;
    ion_skiplist_range *range = (ion_skiplist_range *)udata;
    TSRMLS_FETCH_FROM_CTX(range->thread_ctx);
    if(range->to) {
        compare_function(&result, (zval *)key, range->to TSRMLS_CC);
        if(Z_LVAL(result) > 0) {
            return 1;
        }
    }
    zval_add_ref(value);
    add_next_index_zval(range->result, (zval *)value);
    return 0;
}

static ion_skiplist_range * _ion_skiplist_get_range(int flags, zval *result, zval *to TSRMLS_DC) {
    ion_skiplist_range *range = emalloc(sizeof(ion_skiplist_range));
    memset(range, 0, sizeof(ion_skiplist_range));
    range->flags = flags;
    range->result = result;
    range->to = to;
    TSRMLS_SET_CTX(range->thread_ctx);
    return range;
}

/**
 * Object creating
 */
CLASS_INSTANCE_FREE(ION_Data_SkipList) {
    ion_skiplist *slist = get_object_instance(object, ion_skiplist);
    skiplist_free(slist->list, NULL, php_skiplist_free);
//    efree(slist);
}

CLASS_INSTANCE_INIT(ION_Data_SkipList) {
    ion_skiplist *llist = emalloc(sizeof(ion_skiplist));
    memset(llist, 0, sizeof(ion_skiplist));
    llist->list = skiplist_new(php_skiplist_cmp);
    llist->count = 0;

    RETURN_INSTANCE(ION_Data_SkipList, llist);
}

/**
 * Class methods
 */

typedef int (get_from_end_cb)(struct skiplist *list, void **key, void **value);

void _skiplist_pop(struct skiplist * list, zval * return_value, get_from_end_cb cb, int pop) {
    zval *key = NULL;
    zval *val = NULL;
    if(cb(list, (void **)&key, (void **)&val)) {
        RETURN_NULL();
    }
    array_init(return_value);
    add_next_index_zval(return_value, key);
    add_next_index_zval(return_value, val);
    if(!pop) {
        zval_add_ref(key);
        zval_add_ref(val);
    } else {
        efree(key);
        efree(val);
    }
}

// PHP API
/** public function ION\Data\SkipList::first() : mixed */
CLASS_METHOD(ION_Data_SkipList, first) {
    _skiplist_pop((get_this_instance(ion_skiplist))->list, return_value, skiplist_first, 0);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, first);

/** public function ION\Data\SkipList::last() : mixed */
CLASS_METHOD(ION_Data_SkipList, last) {
    _skiplist_pop((get_this_instance(ion_skiplist))->list, return_value, skiplist_last, 0);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, last);

/** public function ION\Data\SkipList::rPop() : mixed */
CLASS_METHOD(ION_Data_SkipList, rPop) {
    _skiplist_pop((get_this_instance(ion_skiplist))->list, return_value, skiplist_pop_last, 1);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, rPop);

/** public function ION\Data\SkipList::lPop() : mixed */
CLASS_METHOD(ION_Data_SkipList, lPop) {
    _skiplist_pop((get_this_instance(ion_skiplist))->list, return_value, skiplist_pop_first, 1);
}
METHOD_WITHOUT_ARGS(ION_Data_SkipList, lPop);

/** public function ION\Data\SkipList::set(mixed $key, mixed $value) : self */
CLASS_METHOD(ION_Data_SkipList, set) {
    zval * zkey = NULL;
    zval * zvalue = NULL;
    zval * new_key = NULL;
    zval * new_value = NULL;
    zval * zold = NULL;
    ion_skiplist *slist = get_this_instance(ion_skiplist);
    PARSE_ARGS("zz", &zkey, &zvalue);
    new_key = emalloc(sizeof(zval));
    ZVAL_COPY(new_key, zkey);
    new_value = emalloc(sizeof(zval));
    ZVAL_COPY(new_value, zvalue);
    skiplist_set(slist->list, (void *)new_key, (void *)new_value, (void **)&zold);
    if(zold) {
        zval_ptr_dtor(zold);
        efree(zold);
        zval_ptr_dtor(new_key);
        efree(new_key);
    } else {
        slist->count++;
    }
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, set, 2)
    METHOD_ARG(key, 0)
    METHOD_ARG(value, 0)
METHOD_ARGS_END();

/** public function ION\Data\SkipList::add(key, value) : self */
CLASS_METHOD(ION_Data_SkipList, add) {
    zval * zkey = NULL;
    zval * zvalue = NULL;
    zval * new_key = NULL;
    zval * new_value = NULL;
    ion_skiplist *slist = get_this_instance(ion_skiplist);
    PARSE_ARGS("zz", &zkey, &zvalue);
    new_key = emalloc(sizeof(zval));
    ZVAL_COPY(new_key, zkey);
    new_value = emalloc(sizeof(zval));
    ZVAL_COPY(new_value, zvalue);
    skiplist_add(slist->list, (void *)new_key, (void *)new_value);
    slist->count++;
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, add, 2)
    METHOD_ARG(key, false)
    METHOD_ARG(value, false)
METHOD_ARGS_END();


/* public function ION\Data\SkipList::exists(mixed $key) : bool */
CLASS_METHOD(ION_Data_SkipList, exists) {
    zval *key = NULL;
    int result = 0;
    PARSE_ARGS("z", &key);
    result = skiplist_member((get_this_instance(ion_skiplist))->list, (void *)key);
    if(result) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, exists, 1)
    METHOD_ARG(key, false)
METHOD_ARGS_END();


/* public function ION\Data\SkipList::get(mixed $key, bool $all = false) : mixed */
CLASS_METHOD(ION_Data_SkipList, get) {
    zval *key = NULL;
    ion_skiplist * slist = get_this_instance(ion_skiplist);
    zend_bool * all = false;
    PARSE_ARGS("z|b", &key, &all);
    if(all) { // get all values by key
        array_init(return_value);
        ion_skiplist_range *range = _ion_skiplist_get_range(ION_SKIPLIST_RANGE_WITHOUT_KEYS, return_value, key TSRMLS_CC);
        skiplist_iter_from(
                slist->list,
                (void *) key,
                (void *) range,
                php_skiplist_range
        );
        efree(range);
    } else { // get any single value by key
        zval * value = (zval *)skiplist_get((get_this_instance(ion_skiplist))->list, (void *)key);
        if(value == NULL) {
            RETURN_NULL();
        }
        RETVAL_ZVAL(value, 1, 0);
//        efree(value);
    }
}

METHOD_ARGS_BEGIN(ION_Data_SkipList, get, 1)
    METHOD_ARG(key, false)
    METHOD_ARG(all, false)
METHOD_ARGS_END();


/* public function ION\Data\SkipList::count() : int */
CLASS_METHOD(ION_Data_SkipList, count) {
    ion_skiplist *slist = get_this_instance(ion_skiplist);
    RETURN_LONG(skiplist_count(slist->list));
}

METHOD_WITHOUT_ARGS_RETURN_INT(ION_Data_SkipList, count);


/* public function ION\Data\SkipList::toArray() : array */
CLASS_METHOD(ION_Data_SkipList, toArray) {
    array_init(return_value);
    skiplist_iter(
        (get_this_instance(ion_skiplist))->list,
        (void *) return_value,
        php_skiplist_to_array
    );
}

METHOD_WITHOUT_ARGS_RETURN_ARRAY(ION_Data_SkipList, toArray);

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