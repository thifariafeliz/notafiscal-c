#ifndef UTILS_H
#define UTILS_H

#include "errors.h"

typedef struct IntParseResult {
    int value;
    ParseResult status;
} IntParseResult;

IntParseResult parse_int(char *input);

#endif
