#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "list.h"
#include "errors.h"

ListError list_init(List* list, void (*destroy)(void *data)) {
    if (list == NULL) {
        return LE_ARG_IS_NULL;
    }

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    list->destroy = destroy;

    return LE_OK;
}

ListError list_destroy(List *list) {
    if (list == NULL) {
        return LE_ARG_IS_NULL;
    }

    void *data;

    while (list_size(list) > 0) {
        if (list_remove(list, list_tail(list), (void**)&data) == LE_OK && list->destroy != NULL) {
            list->destroy(data);
        }
    }

    memset(list, 0, sizeof(List));

    return LE_OK;
}

ListError list_remove(List *list, Node *node, void **data) {
    if (list == NULL || node == NULL) {
        return LE_ARG_IS_NULL;
    }

    if (list_size(list) == 0) {
        return LE_SIZE_IS_ZERO;
    }

    *data = node->data;

    if (list_is_head(node) == LE_OK) {
        list->head = node->next;
        node->next->prev = NULL;

        if (list_head(list) == NULL) {
            list->tail = NULL;
        }
    }
    else {
        node->prev->next = node->next;

        if (list_next(node) == NULL) {
            list->tail = node->prev;
        }
        else {
            node->next->prev = node->prev;
        }
    }

    free(node);
    return LE_OK;
}

ListError list_ins_next(List *list, Node *node, void *data) {
    if (list == NULL || node == NULL) {
        return LE_ARG_IS_NULL;        
    }

    Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL) {
        return LE_FAILED_MALLOC;
    }

    new_node->data = (void*) data;

    if (list_size(list) == 0) {
        list->head = new_node;
        list->tail = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    }
    else {
        new_node->next = node->next;
        new_node->prev = node;

        if (list_next(new_node) == NULL) {
            list->tail = new_node;
        }
        else {
            node->next->prev = new_node;
        }

        node->next = new_node;
    }

    list->size++;

    return 0;
}

ListError list_ins_prev(List *list, Node *node, void *data) {
    if (list == NULL || node == NULL) {
        return LE_ARG_IS_NULL;
    }

    Node *new_node = malloc(sizeof(*new_node));

    new_node->data = (void*) data;

    if (list_size(list) == 0) {
        list->head = new_node;
        list->tail = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    }
    else {
        new_node->next = node;
        new_node->prev = node->prev;

        if (list_prev(new_node) == NULL) {
            list->head = new_node;
        }
        else {
            node->prev->next = new_node;
        }

        node->prev = new_node;
    }

    list->size++;

    return LE_OK;
}
