#ifndef STRINGO_H
#define STRINGO_H

#include <stddef.h>

typedef struct String {
    char   *data;
    size_t length;
    size_t capacity;
} String;

#endif
