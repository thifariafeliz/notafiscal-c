#ifndef NOTAFISCAL_H
#define NOTAFISCAL_H

#include <stddef.h>

#include "list.h"
#include "stringo.h"

typedef struct DadosCliente {
    Stringo *nome;
    Stringo *endereco;
    Stringo *cpf;
    Stringo *fone;
} DadosCliente;

typedef struct DadosItem {
    Stringo *descricao;

    int codigo;
    int quantidade;
    int preco_total;
    int preco_unitario;
} DadosItem;

typedef struct NotaFiscal {
    List *item;  // List *item receberá elementos do tipo DadosItem
    DadosCliente *cliente;
} NotaFiscal;

typedef struct Caixa {
    List *notas_fiscais;  // List *notas_ficais receberá elementos do tipo NotaFiscal
} Caixa;

void imprimir_notas_fiscais(Caixa *caixa);

#endif
