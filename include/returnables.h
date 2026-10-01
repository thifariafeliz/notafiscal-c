#ifndef ERRORS_H
#define ERRORS_H

typedef struct Item Item;
typedef struct Cliente Cliente;
typedef struct Stringo Stringo;
typedef struct NotaFiscal NotaFiscal;

typedef enum {
    NF_ERR_OK = 0,
    NF_ERR_IO,
    NF_ERR_OVERFLOW,
    NF_ERR_INVALID_ARG,
    NF_ERR_GENERIC_FAIL,  // Used in place of `false`
    NF_ERR_OUT_OF_RANGE,
    NF_ERR_SIZE_IS_ZERO,
    NF_ERR_INVALID_INPUT,
    NF_ERR_INVALID_NODE_LIST,
    NF_ERR_NO_MEMORY_AVAILABLE,
} NFError;

typedef struct ItemResult {
    Item *value;
    NFError status;
} ItemResult;

typedef struct ClienteResult {
    Cliente *value;
    NFError status;
} ClienteResult;

typedef struct StringoResult {
    Stringo *value;
    NFError status;
} StringoResult;

typedef struct NotaFiscalResult {
    NotaFiscal *value;
    NFError status;
} NotaFiscalResult;

typedef struct IntParseResult {
    int value;
    NFError status;
} IntParseResult;

#endif
