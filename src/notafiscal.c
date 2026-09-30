#include <stdio.h>
#include <stdlib.h>

#include "../include/list.h"
#include "../include/notafiscal.h"

void imprimir_notas_fiscais(Caixa *caixa) {
    List* lista_de_nfs = caixa->notas_fiscais;

    Node *node = (Node*) lista_de_nfs->head;

    for (size_t i = 0; i < lista_de_nfs->size; i++) {
        NotaFiscal *nota = (NotaFiscal*) node->data;

        printf("%s, CPF: %s\nEndereço: %s - Telefone: %s\n", nota->cliente->nome->data, nota->cliente->cpf->data,
            nota->cliente->endereco->data, nota->cliente->fone->data);

        List *itens = nota->item;

        Node *item_node = itens->head;
        for (size_t j = 0; j < itens->size; j++) {
            DadosItem *item = item_node->data;

            printf("%d - %s - Preço unitário: %.2f\nQuantidade: %d - Preço Total: %.2f\n", item->codigo, item->descricao->data,
                item->preco_unitario / 100.00, item->quantidade, item->preco_total / 100.00);

            item_node = item_node->next;
        }
        node = node->next;
    }

    return;
}
