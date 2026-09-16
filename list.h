#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#include "errors.h"

typedef struct Node {
    void        *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct List {
    Node   *head;
    Node   *tail;
    size_t size;

    void (*destroy)(void *data);
} List;


ListError list_init(List *list, void (*destroy)(void *data));
ListError list_destroy(List *list);
ListError list_ins_next(List *list, Node *node, void *data);
ListError list_ins_prev(List *list, Node *node, void *data);
ListError list_remove(List *list, Node *node, void **data);

static inline size_t list_size(List *list) {
    return list->size;
}

static inline Node *list_head(List* list) {
    return list->head;
}

static inline Node *list_tail(List *list) {
    return list->tail;
}

static inline ListError list_is_head(Node *node) {
    return node->next == NULL ? LE_OK : LE_FALSE;
}

static inline ListError list_is_tail(Node *node) {
    return node->prev == NULL ? LE_OK : LE_FALSE;
}

static inline void *list_data(Node *node) {
    return node->data;
}

static inline Node *list_next(Node *node) {
    return node->next;
}

static inline Node *list_prev(Node *node) {
    return node->prev;
}

#endif
