#ifndef PION_LINKEDLIST_H
#define PION_LINKEDLIST_H

typedef struct _pion_llist_item pion_llist_item;

// Linked list C API
typedef struct _pion_llist_item {
    pion_llist_item * prev;
    pion_llist_item * next;
    void            * data;
};

typedef struct _pion_llist {
    pion_llist_item * head;
    pion_llist_item * tail;
} pion_llist;


pion_llist *pion_llist_init();
void   pion_llist_free(pion_llist *list);
void   pion_llist_rpush(pion_llist *list, void *data);
void   pion_llist_lpush(pion_llist *list, void *data);
void * pion_llist_lpop(pion_llist *list);
void * pion_llist_rpop(pion_llist *list);

#endif //PION_LINKEDLIST_H
