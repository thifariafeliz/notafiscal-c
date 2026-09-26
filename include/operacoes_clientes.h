#ifndef OPERACOESCLIENTES_H
#define OPERACOESCLIENTES_H

#include "notafiscal.h"
#include "errors.h"

typedef struct DadosClienteResult {
    DadosCliente *cliente;
    StringoError status;
} DadosClienteResult;

int imprimir_clientes(Caixa *caixa);
DadosClienteResult pega_dados_cliente(void);
void dados_cliente_destroy(void *data);

#endif
