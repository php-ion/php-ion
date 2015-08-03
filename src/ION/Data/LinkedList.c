#include "LinkedList.h"
#include <zend_interfaces.h>
#include <ext/spl/spl_iterators.h>

DEFINE_CLASS(ION_Data_LinkedList);

CLASS_INSTANCE_DTOR(ION_Data_LinkedList) {
    IONLinkedList *llist = getInstanceObject(IONLinkedList *);
    zval *item;
    if(llist->count) {
        while(item = pionLListLPop(llist->list)) {
            zval_ptr_dtor(&item);
        }
    }
    pionLListFree(llist->list);
    efree(llist);
}

CLASS_INSTANCE_CTOR(ION_Data_LinkedList) {
    IONLinkedList *llist = emalloc(sizeof(IONLinkedList));
    memset(llist, 0, sizeof(IONLinkedList));
    llist->list = pionLListInit();
    llist->count = 0;
    llist->key = 0;

    RETURN_INSTANCE(ION_Data_LinkedList, llist);
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
    pionLListItem *item;
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
/* public function ION\Dat\LinkedList::rPush(mixed item) : int */
CLASS_METHOD(ION_Data_LinkedList, rPush, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    zval *zitem = NULL;
    PARSE_ARGS("z/", &zitem);

    pionLListRPush(llist->list, zitem);
    zval_add_ref(&zitem);
    RETURN_LONG(++llist->count);
}

METHOD_ARGS_BEGIN(ION_Data_LinkedList, rPush, 1)
    METHOD_ARG(item, 0)
METHOD_ARGS_END();

/* public function ION\Dat\LinkedList::lPush(mixed item) : int */
CLASS_METHOD(ION_Data_LinkedList, lPush, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    zval *zitem = NULL;
    PARSE_ARGS("z/", &zitem);

    pionLListLPush(llist->list, zitem);
    zval_add_ref(&zitem);
    RETURN_LONG(++llist->count);
}

METHOD_ARGS_BEGIN(ION_Data_LinkedList, lPush, 1)
    METHOD_ARG(item, 0)
METHOD_ARGS_END();


/* public function ION\Dat\LinkedList::rPop() : mixed */
CLASS_METHOD(ION_Data_LinkedList, rPop, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    zval *zitem = NULL;
    if(llist->current && !llist->current->next) {
        llist->current = NULL;
    }

    zitem = pionLListRPop(llist->list);
    if(zitem) {
        llist->count--;
        RETURN_ZVAL(zitem, 0, 1);
    } else {
        llist->count = 0;
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, rPop);

/* public function ION\Dat\LinkedList::lPop() : mixed */
CLASS_METHOD(ION_Data_LinkedList, lPop, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    zval *zitem = NULL;
    if(llist->current && !llist->current->prev) {
        if(llist->current->next) {
            llist->current = llist->current->next;
        } else {
            llist->current = NULL;
        }
    }

    zitem = pionLListLPop(llist->list);
    if(zitem) {
        llist->count--;
        RETURN_ZVAL(zitem, 0, 1);
    } else {
        llist->count = 0;
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, lPop);


/* public function ION\Dat\LinkedList::count() : int */
CLASS_METHOD(ION_Data_LinkedList, count, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    RETURN_LONG(llist->count);
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, count);


/* public function ION\Dat\LinkedList::rewind() : void */
CLASS_METHOD(ION_Data_LinkedList, rewind, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    if(llist->list->head) {
        llist->current = llist->list->head;
    } else {
        llist->current = NULL;
    }
    llist->key = 0;
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, rewind);

/* public function ION\Dat\LinkedList::current() : mixed */
CLASS_METHOD(ION_Data_LinkedList, current, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    if(llist->current) {
        zval *item = (zval *)llist->current->data;
        RETURN_ZVAL(item, 0, 0);
    } else {
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, current);


/* public function ION\Dat\LinkedList::key() : mixed */
CLASS_METHOD(ION_Data_LinkedList, key, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    if(llist->current) {
        RETURN_LONG(llist->key);
    } else {
        RETURN_NULL();
    }
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, key);


/* public function ION\Dat\LinkedList::next() : void */
CLASS_METHOD(ION_Data_LinkedList, next, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);
    pionLListItem *item;
    if(llist->current) {
        llist->key++;
        item = llist->current;
        llist->current = item->next;
    }
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, next);


/* public function ION\Dat\LinkedList::valid() : bool */
CLASS_METHOD(ION_Data_LinkedList, valid, ZEND_ACC_PUBLIC) {
    IONLinkedList *llist = getThisInstance(IONLinkedList *);

    RETURN_BOOL(llist->current != NULL);
}

METHOD_WITHOUT_ARGS(ION_Data_LinkedList, valid);


CLASS_METHODS_START(ION_Data_LinkedList)
    METHOD(ION_Data_LinkedList, rPush)
    METHOD(ION_Data_LinkedList, lPush)
    METHOD(ION_Data_LinkedList, rPop)
    METHOD(ION_Data_LinkedList, lPop)
    METHOD(ION_Data_LinkedList, count)
    METHOD(ION_Data_LinkedList, rewind)
    METHOD(ION_Data_LinkedList, current)
    METHOD(ION_Data_LinkedList, key)
    METHOD(ION_Data_LinkedList, next)
    METHOD(ION_Data_LinkedList, valid)
CLASS_METHODS_END;

PHP_MINIT_FUNCTION(ION_Data_LinkedList) {
    PION_REGISTER_CLASS(ION_Data_LinkedList, "ION\\Data\\LinkedList");
    zend_class_implements(CE(ION_Data_LinkedList) TSRMLS_CC, 1, zend_ce_iterator);
    zend_class_implements(CE(ION_Data_LinkedList) TSRMLS_CC, 1, spl_ce_Countable);
    CE(ION_Data_LinkedList)->get_iterator = ion_llist_get_iterator;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_Data_LinkedList) {
    return SUCCESS;
}
