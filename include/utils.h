#ifndef UTILS_H
#define UTILS_H

#include "errors.h"

typedef struct IntParseResult {
    int value;
    ParseResult status;
} IntParseResult;

IntParseResult take_int();
IntParseResult parse_int(char *input);
void buffer_flush();

#endif
