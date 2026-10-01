#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "../include/utils.h"
#include "../include/returnables.h"

IntParseResult parse_int(char *input) {
    if (input == NULL) {
        return (IntParseResult){.value = -1, .status = NF_ERR_INVALID_ARG};
    }

    errno = 0;
    char *end = NULL;
    long int number = strtol(input, &end, 10);

    if (end == input) {
        return (IntParseResult){.value = -1, .status = NF_ERR_INVALID_INPUT};
    }

    if (*end != '\n' && *end != '\0') {
        return (IntParseResult){.value = -1, .status = NF_ERR_INVALID_INPUT};
    }

    if (errno == ERANGE || number < INT_MIN || number > INT_MAX) {
        return (IntParseResult){.value = -1, .status = NF_ERR_OVERFLOW};
    }

    return (IntParseResult){.value = (int) number, .status = NF_ERR_OK};
}
