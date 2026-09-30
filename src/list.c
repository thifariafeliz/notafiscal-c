#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/list.h"
#include "../include/returnables.h"

NFError list_init(List* list, void (*destroy)(void *data)) {
    if (list == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    list->destroy = destroy;

    return NF_ERR_OK;
}

NFError list_destroy(List *list) {
    if (list == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    void *data;

    while (list_size(list) > 0) {
        if (list_remove(list, list_tail(list), (void**)&data) == NF_ERR_OK && list->destroy != NULL) {
            list->destroy(data);
        }
    }

    memset(list, 0, sizeof(List));

    return NF_ERR_OK;
}

NFError list_remove(List *list, Node *node, void **data) {
    if (list == NULL || node == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if (list_size(list) == 0) {
        return NF_ERR_SIZE_IS_ZERO;
    }

    if (list_head(list) == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    *data = node->data;

    if (list_is_head(node) == NF_ERR_OK) {
        list->head = node->next;
        // node->next->prev = NULL;

        if (list_head(list) == NULL) {
            list->tail = NULL;
        }
        else {
            node->next->prev = NULL;
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

    list->size--;

    return NF_ERR_OK;
}

NFError list_ins_next(List *list, Node *node, void *data) {
    if (list == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if (list_size(list) != 0 && node == NULL) {
        return NF_ERR_GENERIC_FAIL;
    }

    Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL) {
        return NF_ERR_NO_MEMORY_AVAILABLE;
    }

    new_node->data = (void*) data;

    if (list_size(list) == 0) {
        list->head = new_node;
        list->tail = new_node;
        list->head->prev = NULL;
        list->tail->next = NULL;
    }
    else {
        new_node->next = node->next;
        new_node->prev = node;

        if (list_next(node) == NULL) {
            list->tail = new_node;
        }
        else {
            node->next->prev = new_node;
        }

        node->next = new_node;
    }

    list->size++;

    return NF_ERR_OK;
}

NFError list_ins_prev(List *list, Node *node, void *data) {
    if (list == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if (list_size(list) != 0 && node == NULL) {
        return NF_ERR_GENERIC_FAIL;
    }

    Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL) {
        return NF_ERR_NO_MEMORY_AVAILABLE;
    }

    new_node->data = (void*) data;

    if (list_size(list) == 0) {
        list->head = new_node;
        list->tail = new_node;
        list->head->prev = NULL;
        list->tail->next = NULL;
    }
    else {
        new_node->next = node;
        new_node->prev = node->prev;

        if (list_prev(node) == NULL) {
            list->head = new_node;
        }
        else {
            node->prev->next = new_node;
        }

        node->prev = new_node;
    }

    list->size++;

    return NF_ERR_OK;
}
