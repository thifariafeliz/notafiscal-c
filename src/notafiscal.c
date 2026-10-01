#include <stdio.h>
#include <stdlib.h>

#include "../include/list.h"
#include "../include/notafiscal.h"
#include "../include/operacoes_clientes.h"

NFError imprimir_notas_fiscais(List *lista_nfs) {
    if (lista_nfs == NULL || lista_nfs->head == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if (lista_nfs->size == 0) {
        return NF_ERR_SIZE_IS_ZERO;
    }

    Node *node = lista_nfs->head;

    for (size_t i = 0; i < lista_nfs->size; i++) {
        if (node->list != lista_nfs) {
            return NF_ERR_INVALID_NODE_LIST;
        }

        NotaFiscal *nota_fiscal = (NotaFiscal*) node->data;
        if (nota_fiscal == NULL) {
            continue;
        }

        NFError print_result = cliente_imprimir(nota_fiscal);

        if (print_result != NF_ERR_OK) {
            return print_result;
        }
    }

    return NF_ERR_OK;
}
