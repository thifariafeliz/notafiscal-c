#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>

#include "../include/utils.h"
#include "../include/stringo.h"
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

NFError pega_int(int *ret) {
    for (;;) {
        StringoResult input = stringo_get_input();
        if (input.status != NF_ERR_OK) {
            return input.status;
        }

        IntParseResult int_result = parse_int(input.value->data);
        if (int_result.status != NF_ERR_OK) {
            if (int_result.status == NF_ERR_INVALID_ARG) {
                stringo_destroy(input.value);
                ret = NULL;
                return int_result.status;
            }

            stringo_destroy(input.value);
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        *ret = int_result.value;
        stringo_destroy(input.value);
        break;
    }

    return NF_ERR_OK;
}
