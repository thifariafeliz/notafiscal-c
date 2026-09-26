#ifndef ERRORS_H
#define ERRORS_H

typedef enum ListError {
    LE_OK = 0,
    LE_ARG_IS_NULL,
    LE_SIZE_IS_ZERO,
    LE_GENERIC_FAIL,
    LE_FAILED_MALLOC,
} ListError;

typedef enum StringoError {
    SE_OK = 0,
    SE_EARLY_EOF,
    SE_ARG_IS_NULL,
    SE_GENERIC_FAIL,
    SE_FAILED_MALLOC,
} StringoError;

typedef enum ParseResult {
    PARSE_OK = 0,
    PARSE_EMPTY,
    PARSE_INVALID,
    PARSE_OVERFLOW,
    PARSE_ARG_IS_NULL,
    PARSE_GENERIC_FAIL,
} ParseResult;

typedef enum DadosItemResult {
    DI_OK = 0,
    DI_INT_ERROR,
    DI_GENERIC_FAIL,
    DI_STRINGO_ERROR,
    DI_FAILED_MALLOC,
} DadosItemResult;

#endif
