#include "linkedlist.h"
#include <php.h>


pion_llist *pion_llist_init() {
    pion_llist *list = emalloc(sizeof(pion_llist));
    list->tail = list->head = NULL;
    return list;
}

void pion_llist_free(pion_llist *list) {
    efree(list);
}

void pion_llist_rpush(pion_llist * list, void * data) {
    pion_llist_item * item = emalloc(sizeof(pion_llist_item));
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

void pion_llist_lpush(pion_llist * list, void * data) {
    pion_llist_item * item = emalloc(sizeof(pion_llist_item));
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

void * pion_llist_lpop(pion_llist *list) {
    pion_llist_item *item;
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

void * pion_llist_rpop(pion_llist *list) {
    pion_llist_item *item;
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