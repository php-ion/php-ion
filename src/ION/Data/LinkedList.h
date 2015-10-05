#ifndef ION_LINKEDLIST_H
#define ION_LINKEDLIST_H

#include <php.h>
#include <zend.h>
#include "../../pion.h"
#include "../../pion/linkedlist.h"

// LinkedList PHP object
typedef struct _php_linked_list {
    zend_object std;
    pion_llist *list;
    long        count;
    long        key;
    pion_llist_item *current;
} ion_linked_list;


PHP_MINIT_FUNCTION(ION_Data_LinkedList);

#endif //ION_LINKEDLIST_H
