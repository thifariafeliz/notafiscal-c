#ifndef NOTAFISCAL_H
#define NOTAFISCAL_H

#include "stringo.h"

typedef struct DadosCliente {
    String nome;
    String endereco;
    String cpf;
    String fone;
} DadosCliente;

typedef struct DadosItem {
    String descricao;
    int codigo;
    int quantidade;
    int preco_unitario;
    int preco_total;
} DadosItem;

typedef struct NotaFiscal {
    DadosCliente cliente;
    DadosItem    item;
} NotaFiscal;

#endif
