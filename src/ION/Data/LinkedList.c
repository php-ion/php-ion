#include "LinkedList.h"
#include "../../framework.h"
#include "zend_interfaces.h"
#include "ext/spl/spl_iterators.h"
#include <ext/spl/spl_functions.h>

DEFINE_CLASS(ION_Data_LinkedList);


LList* ion_llist_ctor() {
    LList *list = emalloc(sizeof(LList));
    list->tail = list->head = NULL;
    return list;
}

void ion_llist_dtor(LList *list) {
    efree(list);
}

void ion_llist_rpush(LList *list, void *data) {
    LListItem *item = emalloc(sizeof(LListItem));
    item->data = data;
    item->next  = NULL;
    if(list->tail) { // append item
        item->prev = list->tail;
        list->tail->next = item;
        list->tail = item;
    } else { // push first item
        item->prev = NULL;
        list->tail = list->head = item;
    }
}

void ion_llist_lpush(LList *list, void *data) {
    LListItem *item = emalloc(sizeof(LListItem));
    item->data = data;
    item->prev  = NULL;
    if(list->head) { // prepend item
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
    } else { // push first item
        item->next = NULL;
        list->tail = list->head = item;
    }

}

void* ion_llist_lpop(LList *list) {
    LListItem *item;
    void *data = NULL;
    if(list->head == NULL) {
        return NULL;
    }
    item = list->head;
    if(item->next) { // pop item
        list->head = item->next;
        item->next->prev = NULL;
    } else { // pop last item
        list->head = list->tail = NULL;
    }
    data = item->data;
    efree(item);
    return data;
}

void* ion_llist_rpop(LList *list) {
    LListItem *item;
    void *data = NULL;
    if(list->tail == NULL) {
        return NULL;
    }
    item = list->tail;
    if(item->prev) { // pop item
        list->tail = item->prev;
        item->prev->next = NULL;
    } else { // pop last item
        list->head = list->tail = NULL;
    }
    data = item->data;
    efree(item);
    return data;
}

static void _ion_llist_dtor(void *object TSRMLS_DC) {
    IONLinkedList *llist = (IONLinkedList *) object;
    zval *item;
    if(llist->count) {
        while(item = ion_llist_lpop(llist->list)) {
            zval_ptr_dtor(&item);
        }
    }
    ion_llist_dtor(llist->list);
    efree(llist);
}

static zend_object_value _ion_llist_ctor(zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    IONLinkedList *llist = emalloc(sizeof(IONLinkedList));
    memset(llist, 0, sizeof(IONLinkedList));
    llist->list = ion_llist_ctor();
    llist->count = 0;
    llist->key = 0;
    OBJECT_INIT(retval, ION_Data_LinkedList, llist, _ion_llist_dtor);
    return retval;
}

typedef struct {
    zend_user_iterator  intern;
    IONLinkedList      *object;
} ion_llist_it;

static void ion_llist_it_dtor(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
    ion_llist_it *iterator = (ion_llist_it *)iter;

    zend_user_it_invalidate_current(iter TSRMLS_CC);
    zval_ptr_dtor((zval**)&iterator->intern.it.data);

    efree(iterator);
}

static int ion_llist_it_valid(zend_object_iterator *iter TSRMLS_DC) {
    ion_llist_it       *iterator = (ion_llist_it *)iter;
    IONLinkedList      *object   = iterator->object;
    if(object->current == NULL) {
        return FAILURE;
    } else {
        return SUCCESS;
    }
}

static void ion_llist_it_get_current_data(zend_object_iterator *iter, zval ***data TSRMLS_DC) {
    ion_llist_it  *iterator = (ion_llist_it *)iter;
    IONLinkedList *object   = iterator->object;
    if(object->current == NULL) {
        *data = NULL;
    } else {
        *data = (zval **)&object->current->data;
    }
}

static void ion_llist_it_get_current_key(zend_object_iterator *iter, zval *key TSRMLS_DC) {
    ion_llist_it   *iterator = (ion_llist_it *)iter;
    IONLinkedList  *object   = iterator->object;
    if(object->current == NULL) {
        ZVAL_NULL(key);
    } else {
        ZVAL_LONG(key, object->key);
    }
}

static void ion_llist_it_move_forward(zend_object_iterator *iter TSRMLS_DC) {
    ion_llist_it  *iterator = (ion_llist_it *)iter;
    IONLinkedList *object   = iterator->object;
    LListItem     *item;
    if(object->current != NULL) {
        object->key++;
        item = object->current;
        if(item->next) {
            object->current = item->next;
        } else {
            object->current = NULL;
        }
    }
}

static void ion_llist_it_rewind(zend_object_iterator *iter TSRMLS_DC) {
    ion_llist_it  *iterator = (ion_llist_it *)iter;
    IONLinkedList *object   = iterator->object;

    if(object->list->head) {
        object->current = object->list->head;
    } else {
        object->current = NULL;
    }
    object->key = 0;
}

/* iterator handler table */
zend_object_iterator_funcs ion_llist_it_funcs = {
    ion_llist_it_dtor,
    ion_llist_it_valid,
    ion_llist_it_get_current_data,
    ion_llist_it_get_current_key,
    ion_llist_it_move_forward,
    ion_llist_it_rewind
};

zend_object_iterator *ion_llist_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC) {
    ion_llist_it    *iterator;
    IONLinkedList   *llist = (IONLinkedList*)zend_object_store_get_object(object TSRMLS_CC);

    iterator     = emalloc(sizeof(ion_llist_it));

    Z_ADDREF_P(object);
    iterator->intern.it.data = (void*)object;
    iterator->intern.it.funcs = &ion_llist_it_funcs;
    iterator->intern.ce = ce;
    iterator->intern.value = NULL;
    iterator->object = llist;

    return (zend_object_iterator*)iterator;
}

// PHP API
PHP_METHOD(ION_Data_LinkedList, rPush) {
    IONLinkedList *llist = FETCH_HANDLE();
    zval *zitem = NULL;
    PARSE_ARGS("z/", &zitem);

    ion_llist_rpush(llist->list, zitem);
    zval_add_ref(&zitem);
    RETURN_LONG(++llist->count);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_rPush, 0, 0, 1)
                ZEND_ARG_INFO(0, item)
ZEND_END_ARG_INFO();


PHP_METHOD(ION_Data_LinkedList, lPush) {
    IONLinkedList *llist = FETCH_HANDLE();
    zval *zitem = NULL;
    PARSE_ARGS("z/", &zitem);

    ion_llist_lpush(llist->list, zitem);
    zval_add_ref(&zitem);
    RETURN_LONG(++llist->count);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_lPush, 0, 0, 1)
    ZEND_ARG_INFO(0, item)
ZEND_END_ARG_INFO();

PHP_METHOD(ION_Data_LinkedList, rPop) {
    IONLinkedList *llist = FETCH_HANDLE();
    zval *zitem = NULL;
    if(llist->current && !llist->current->next) {
        llist->current = NULL;
    }

    zitem = ion_llist_rpop(llist->list);
    if(zitem) {
        llist->count--;
        RETURN_ZVAL(zitem, 0, 1);
    } else {
        llist->count = 0;
        RETURN_NULL();
    }
}

PHP_METHOD(ION_Data_LinkedList, lPop) {
    IONLinkedList *llist = FETCH_HANDLE();
    zval *zitem = NULL;
    if(llist->current && !llist->current->prev) {
        if(llist->current->next) {
            llist->current = llist->current->next;
        } else {
            llist->current = NULL;
        }
    }

    zitem = ion_llist_lpop(llist->list);
    if(zitem) {
        llist->count--;
        RETURN_ZVAL(zitem, 0, 1);
    } else {
        llist->count = 0;
        RETURN_NULL();
    }
}

PHP_METHOD(ION_Data_LinkedList, count) {
    IONLinkedList *llist = FETCH_HANDLE();
    RETURN_LONG(llist->count);
}

PHP_METHOD(ION_Data_LinkedList, rewind) {
    IONLinkedList *llist = FETCH_HANDLE();
    if(llist->list->head) {
        llist->current = llist->list->head;
    } else {
        llist->current = NULL;
    }
    llist->key = 0;
}

PHP_METHOD(ION_Data_LinkedList, current) {
    IONLinkedList *llist = FETCH_HANDLE();
    if(llist->current) {
        zval *item = (zval *)llist->current->data;
        RETURN_ZVAL(item, 0, 0);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ION_Data_LinkedList, key) {
    IONLinkedList *llist = FETCH_HANDLE();
    if(llist->current) {
        RETURN_LONG(llist->key);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ION_Data_LinkedList, next) {
    IONLinkedList *llist = FETCH_HANDLE();
    LListItem     *item;
    if(llist->current) {
        llist->key++;
        item = llist->current;
        llist->current = item->next;
    }
}

PHP_METHOD(ION_Data_LinkedList, valid) {
    IONLinkedList *llist = FETCH_HANDLE();

    RETURN_BOOL(llist->current != NULL);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_noargs, 0, 0, 0)
ZEND_END_ARG_INFO();

CLASS_METHODS_START(ION_Data_LinkedList)
ZEND_ME_ARG(ION_Data_LinkedList, rPush,  ZEND_ACC_PUBLIC)
ZEND_ME_ARG(ION_Data_LinkedList, lPush,  ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, rPop,   ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, lPop,   ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, count,  ZEND_ACC_PUBLIC)
/* Iterator */
ZEND_ME_NOARG(ION_Data_LinkedList, rewind,  ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, current, ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, key,     ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, next,    ZEND_ACC_PUBLIC)
ZEND_ME_NOARG(ION_Data_LinkedList, valid,   ZEND_ACC_PUBLIC)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Data_LinkedList) {
    REGISTER_CLASS(ION_Data_LinkedList, "ION\\Data\\LinkedList", _ion_llist_ctor);
    zend_class_implements(CE(ION_Data_LinkedList) TSRMLS_CC, 1, zend_ce_iterator);
    zend_class_implements(CE(ION_Data_LinkedList) TSRMLS_CC, 1, spl_ce_Countable);
    CE(ION_Data_LinkedList)->get_iterator = ion_llist_get_iterator;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Data_LinkedList) {
    return SUCCESS;
}
