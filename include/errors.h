#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    NF_ERR_OK = 0,
    NF_ERR_IO,
    NF_ERR_OUT_OF_RANGE,
    NF_ERR_INVALID_INPUT,
    NF_ERR_NO_MEMORY_AVAILABLE,

} NFError;

#endif
