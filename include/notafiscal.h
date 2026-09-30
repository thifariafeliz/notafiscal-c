#ifndef NOTAFISCAL_H
#define NOTAFISCAL_H

#include <stddef.h>

#include "stringo.h"
#include "list.h"

typedef struct Cliente {
    Stringo *nome;
    Stringo *endereco;
    Stringo *cpf;
    Stringo *fone;
} Cliente;

typedef struct Item {
    Stringo *descricao;

    int codigo;
    int quantidade;
    int preco_total;
    int preco_unitario;
} Item;

typedef struct NotaFiscal {
    List *itens;  // List *item receberá elementos do tipo DadosItem
    Cliente *cliente;
} NotaFiscal;

#endif
