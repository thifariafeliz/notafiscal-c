#ifndef LIST_H
#define LIST_H

#include <stddef.h>

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



#endif
