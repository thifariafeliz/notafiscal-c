#ifndef ERRORS_H
#define ERRORS_H

typedef enum ListError {
    LE_OK = 0,
    LE_FALSE,
    LE_ARG_IS_NULL,
    LE_SIZE_IS_ZERO,
    LE_FAILED_MALLOC,
} ListError;

typedef enum StringoError {
    SE_OK = 0,
    SE_FALSE,
    SE_EARLY_EOF,
    SE_ARG_IS_NULL,
    SE_FAILED_MALLOC,
} StringoError;

#endif
