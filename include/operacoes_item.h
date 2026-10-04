#ifndef OPERACOES_ITEM_H
#define OPERACOES_ITEM_H

#include "notafiscal.h"
#include "returnables.h"

ItemResult item_pegar_dados();
void item_destroy(void *data);
NFError item_imprimir(Item *item);

#endif
