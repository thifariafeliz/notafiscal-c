#ifndef OPERACOES_ITEM_H
#define OPERACOES_ITEM_H

#include "errors.h"
#include "notafiscal.h"

typedef struct OpDadosItemResult {
    DadosItem *item;
    DadosItemResult status;
} OpDadosItemResult;

OpDadosItemResult pega_dados_item();
void dados_item_destroy(void *data);

#endif
