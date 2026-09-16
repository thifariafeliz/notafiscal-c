#ifndef STRINGO_H
#define STRINGO_H

#include <stddef.h>

#include "errors.h"

typedef struct Stringo {
    char   *data;
    size_t length;
    size_t capacity;
} Stringo;

StringoError stringo_get_input(Stringo *input);

#endif
