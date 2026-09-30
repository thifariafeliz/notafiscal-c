#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "../include/utils.h"
#include "../include/errors.h"

IntParseResult parse_int(char *input) {
    if (input == NULL) {
        return (IntParseResult){.value = -1, .status = PARSE_ARG_IS_NULL};
    }

    errno = 0;
    char *end = NULL;
    long int number = strtol(input, &end, 10);

    if (end == input) {
        return (IntParseResult){.value = -1, .status = PARSE_EMPTY};
    }

    if (*end != '\n' && *end != '\0') {
        return (IntParseResult){.value = -1, .status = PARSE_INVALID};
    }

    if (errno == ERANGE || number < INT_MIN || number > INT_MAX) {
        return (IntParseResult){.value = -1, .status = PARSE_OVERFLOW};
    }

    return (IntParseResult){.value = (int) number, .status = PARSE_OK};
}
