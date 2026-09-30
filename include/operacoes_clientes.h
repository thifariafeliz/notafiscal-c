#ifndef OPERACOESCLIENTES_H
#define OPERACOESCLIENTES_H

#include "notafiscal.h"
#include "returnables.h"

void cliente_destroy(void *data);
ClienteResult cliente_pegar_dados(void);
NFError cliente_imprimir(NotaFiscal *nf);
NFError cliente_add_item(NotaFiscal *nf, Item *item);
NotaFiscalResult cliente_procurar(List *lista_nfs, Stringo *cpf);

#endif
