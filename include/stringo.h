#ifndef STRINGO_H
#define STRINGO_H

#include <stddef.h>

#include "returnables.h"

typedef struct Stringo {
    char   *data;
    size_t length;
    size_t capacity;
} Stringo;

StringoResult stringo_initialize();
StringoResult stringo_get_input();
void stringo_destroy(Stringo *str);

static inline char *stringo_data(Stringo *stringo) {
    return stringo->data;
}

static inline size_t stringo_length(Stringo *stringo) {
    return stringo->length;
}

static inline size_t stringo_capacity(Stringo *stringo) {
    return stringo->capacity;
}

#endif
