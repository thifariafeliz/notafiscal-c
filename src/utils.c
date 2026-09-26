#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "../include/utils.h"
#include "../include/errors.h"

IntParseResult take_int() {
    char input[100];

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return (IntParseResult){.value = -1, .status = PARSE_GENERIC_FAIL};
    }

    IntParseResult result = parse_int(input);

    switch (result.status) {
        case PARSE_OK:
            break;
        case PARSE_EMPTY:
            return (IntParseResult){.value = -1, .status = PARSE_EMPTY};
        case PARSE_INVALID:
            return (IntParseResult){.value = -1, .status = PARSE_INVALID};

        case PARSE_OVERFLOW:
            return (IntParseResult){.value = -1, .status = PARSE_OVERFLOW};

        case PARSE_GENERIC_FAIL:
            return (IntParseResult){.value = -1, .status = PARSE_GENERIC_FAIL};

        case PARSE_ARG_IS_NULL:
            return (IntParseResult){.value = -1, .status = PARSE_ARG_IS_NULL};
    }

    return (IntParseResult){.value = result.value, .status = PARSE_OK};
}

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

void buffer_flush() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
