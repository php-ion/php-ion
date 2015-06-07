#ifndef ION_LINKEDLIST_H
#define ION_LINKEDLIST_H

#include <php.h>
#include <zend.h>
#include "../../framework.h"

// Linked list C API
typedef struct _linked_list_item {
    struct _linked_list_item *prev;
    struct _linked_list_item *next;
    void                     *data;
} LListItem;

typedef struct _linked_list {
    LListItem   *head;
    LListItem   *tail;
} LList;


LList* ion_llist_ctor();
void ion_llist_dtor(LList *list);
void ion_llist_rpush(LList *list, void *data);
void ion_llist_lpush(LList *list, void *data);
void* ion_llist_lpop(LList *list);
void* ion_llist_rpop(LList *list);

// LinkedList PHP object
typedef struct _php_linkedlist {
    zend_object std;
    LList       *list;
    long        count;
    long        key;
    LListItem   *current;
} IONLinkedList;


PHP_MINIT_FUNCTION(ION_Data_LinkedList);
DEFINE_CLASS(ION_Data_LinkedList);

PHP_METHOD(ION_Data_LinkedList, rPush);
PHP_METHOD(ION_Data_LinkedList, lPush);
PHP_METHOD(ION_Data_LinkedList, rPop);
PHP_METHOD(ION_Data_LinkedList, lPop);
PHP_METHOD(ION_Data_LinkedList, count);
PHP_METHOD(ION_Data_LinkedList, rewind);
PHP_METHOD(ION_Data_LinkedList, current);
PHP_METHOD(ION_Data_LinkedList, key);
PHP_METHOD(ION_Data_LinkedList, next);
PHP_METHOD(ION_Data_LinkedList, valid);

#endif //ION_LINKEDLIST_H
