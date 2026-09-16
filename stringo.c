#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringo.h"

StringoReturnResult stringo_initialize() {
    Stringo *stringo = malloc(sizeof(*stringo));
    if (stringo == NULL) {
        return (StringoReturnResult){.string = NULL, .status = SE_FAILED_MALLOC};
    }

    stringo->length = 0;
    stringo->capacity = 16;

    stringo->data = malloc(stringo->capacity);
    if (stringo->data == NULL) {
        stringo->capacity = 0;
        return (StringoReturnResult){.string = NULL, .status = SE_FAILED_MALLOC};
    }

    return (StringoReturnResult){.string = stringo, .status = SE_OK};
}

StringoReturnResult stringo_get_input() {
    StringoReturnResult input = stringo_initialize();
    if (input.status != SE_OK) {
        return (StringoReturnResult){.string = NULL, .status = SE_FAILED_MALLOC};
    }

    int c;

    while (1) {
        c = getchar();

        if (c == EOF) { 
            if (input.string->length == 0) {
                free(input.string->data);
                free(input.string);
                return (StringoReturnResult){.string = NULL, .status = SE_EARLY_EOF};
            }
            break;
        }

        if (c == '\n') {
            break;
        }

        if (stringo_length(input.string) + 1 >= stringo_capacity(input.string)) {
            input.string->capacity *= 2;

            char *temp = realloc(input.string->data, input.string->capacity);
            if (temp == NULL) {
                free(input.string->data);
                free(input.string);
                return (StringoReturnResult){.string = NULL, .status = SE_FAILED_MALLOC};
            }
            
            input.string->data = temp;
        }
        
        input.string->data[input.string->length++] = (char) c;
    }

    input.string->data[stringo_length(input.string)] = '\0'; 

    return (StringoReturnResult){.string = input.string, .status = SE_OK};
}

StringoError stringo_trim_capacity(Stringo *string) {
    if (string == NULL || string->data == NULL) {
        return SE_ARG_IS_NULL;
    }

    char *temp = realloc(stringo_data(string), stringo_length(string) + 1);
    if (temp == NULL) {
        return SE_FAILED_MALLOC;
    }

    return SE_OK;
}
