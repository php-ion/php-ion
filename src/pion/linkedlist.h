#ifndef PION_LINKEDLIST_H
#define PION_LINKEDLIST_H

// Linked list C API
typedef struct _pionLListItem {
    struct _pionLListItem *prev;
    struct _pionLListItem *next;
    void                       *data;
} pionLListItem;

typedef struct _pionLList {
    pionLListItem *head;
    pionLListItem *tail;
} pionLList;


pionLList *pionLListInit();
void  pionLListFree(pionLList *list);
void  pionLListRPush(pionLList *list, void *data);
void  pionLListLPush(pionLList *list, void *data);
void* pionLListLPop(pionLList *list);
void* pionLListRPop(pionLList *list);

#endif //PION_LINKEDLIST_H
