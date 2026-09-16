#ifndef STRINGO_H
#define STRINGO_H

#include <stddef.h>

#include "errors.h"

typedef struct Stringo {
    char   *data;
    size_t length;
    size_t capacity;
} Stringo;

typedef struct StringoReturnResult {
    Stringo *string;
    StringoError status;
} StringoReturnResult;

StringoReturnResult stringo_initialize();
StringoReturnResult stringo_get_input();

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
