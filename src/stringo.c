#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "../include/stringo.h"

StringoResult stringo_initialize() {
    Stringo *stringo = malloc(sizeof(*stringo));
    if (stringo == NULL) {
        return (StringoResult){.value = NULL, .status = NF_ERR_NO_MEMORY_AVAILABLE};
    }

    stringo->length = 0;
    stringo->capacity = 16;

    stringo->data = malloc(stringo->capacity);
    if (stringo->data == NULL) {
        stringo->capacity = 0;
        free(stringo);
        return (StringoResult){.value = NULL, .status = NF_ERR_NO_MEMORY_AVAILABLE};
    }

    return (StringoResult){.value = stringo, .status = NF_ERR_OK};
}

StringoResult stringo_get_input() {
    StringoResult input = stringo_initialize();
    if (input.status != NF_ERR_OK) {
        return (StringoResult){.value = NULL, .status = input.status};
    }

    int c;

    while (1) {
        c = getchar();

        if (c == EOF) {
            if (input.value->length == 0) {
                free(input.value->data);
                free(input.value);
                return (StringoResult){.value = NULL, .status = NF_ERR_IO};
            }
            break;
        }

        if (c == '\n') {
            break;
        }

        if (stringo_length(input.value) + 1 >= stringo_capacity(input.value)) {
            input.value->capacity *= 2;

            char *temp = realloc(input.value->data, input.value->capacity);
            if (temp == NULL) {
                free(input.value->data);
                free(input.value);
                return (StringoResult){.value = NULL, .status = NF_ERR_NO_MEMORY_AVAILABLE};
            }

            input.value->data = temp;
        }

        input.value->data[input.value->length++] = (char) c;
    }

    input.value->data[stringo_length(input.value)] = '\0';

    return (StringoResult){.value = input.value, .status = NF_ERR_OK};
}

// StringoError stringo_trim_capacity(Stringo *string) {
//     if (string == NULL || string->data == NULL) {
//         return SE_ARG_IS_NULL;
//     }

//     char *temp = realloc(stringo_data(string), stringo_length(string) + 1);
//     if (temp == NULL) {
//         return SE_FAILED_MALLOC;
//     }

//     return SE_OK;
// }

void stringo_destroy(Stringo *str) {
    if (str == NULL) {
        return;
    }

    if (str->data == NULL) {
        str->length = 0;
        str->capacity = 0;
        free(str);
        str = NULL;
        return;
    }

    free(str->data);
    str->length = 0;
    str->capacity = 0;
    str->data = NULL;

    free(str);
    str = NULL;

    return;
}
