#include <stdio.h>
#include <stdlib.h>

#include "../include/list.h"
#include "../include/notafiscal.h"
#include "../include/operacoes_item.h"
#include "../include/operacoes_clientes.h"

NFError imprimir_notas_fiscais(List *lista_nfs) {
    if (lista_nfs == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if (lista_nfs->head == NULL && lista_nfs->size == 0) {
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

        printf("\n================================================================================\n");
        NFError print_result = cliente_imprimir(nota_fiscal);

        if (print_result != NF_ERR_OK) {
            return print_result;
        }

        if (nota_fiscal->itens->size != 0) {
            printf("\nProdutos:\n");
            Node *item_node = nota_fiscal->itens->head;
            for (size_t j = 0; j < nota_fiscal->itens->size; j++) {
                if (item_node == NULL) {
                    break;
                }

                Item *item = (Item*) item_node->data;
                if (item_imprimir(item) != NF_ERR_OK) {
                    break;
                }

                item_node = item_node->next;
            }
        }

        printf("\n================================================================================\n");
        node = node->next;
    }

    return NF_ERR_OK;
}

void nfs_destroy(void *data) {
    if (data == NULL) {
        return;
    }

    NotaFiscal *nf = (NotaFiscal*)data;
    if (nf->cliente != NULL) {
        cliente_destroy(nf->cliente);
        free(nf->cliente);
    }

    if (nf->itens != NULL) {
        list_destroy(nf->itens);
        free(nf->itens);
   }

    free(nf);

    return;
}
