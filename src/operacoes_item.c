#include <stdio.h>
#include <stdlib.h>

#include "../include/utils.h"
#include "../include/operacoes_item.h"

void item_destroy(void *data) {
    if (data == NULL) {
        return;
    }

    Item *item = (Item*) data;

    stringo_destroy(item->descricao);
    free(item);
}

ItemResult item_pegar_dados(void) {
    Item *item = calloc(1, sizeof(*item));
    if (item == NULL) {
        return (ItemResult){.value = NULL, .status = NF_ERR_NO_MEMORY_AVAILABLE};
    }

    printf("Insira a descrição do item: ");
    StringoResult input_result = stringo_get_input();
    if (input_result.status != NF_ERR_OK) {
        if (input_result.status != NF_ERR_NO_MEMORY_AVAILABLE) {
            stringo_destroy(input_result.value);
        }
        item_destroy(item);
        return (ItemResult){.value = NULL, .status = input_result.status};
    }
    item->descricao = input_result.value;

    printf("Insira o código do item: ");
    int codigo = 0;
    NFError int_result = pega_int(&codigo);
    if (int_result != NF_ERR_OK) {
        item_destroy(item);
        return (ItemResult){.value = NULL, .status = int_result};
    }
    item->codigo = codigo;

    printf("Insira o preço unitário do item: ");
    int preco_unitario = 0;
    int_result = pega_int(&preco_unitario);
    if (int_result != NF_ERR_OK) {
        item_destroy(item);
        return (ItemResult){.value = NULL, .status = int_result};
    }
    item->preco_unitario = preco_unitario;

    printf("Insira a quantidade de itens: ");
    int quantidade = 0;
    int_result = pega_int(&quantidade);
    if (int_result != NF_ERR_OK) {
        item_destroy(item);
        return (ItemResult){.value = NULL, .status = int_result};
    }
    item->quantidade = quantidade;

    item->preco_total = item->preco_unitario * item->quantidade;

    return (ItemResult){.value = item, .status = NF_ERR_OK};
}

NFError item_imprimir(Item *item) {
    if (item == NULL || item->descricao == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if (item->descricao->data != NULL) {
        printf("%s", item->descricao->data);
    }

    printf(" - %d\nPreço unitário: %.2f\nQuantidade: %d\nPreço total: %.2f\n\n",
        item->codigo,
        ((float)item->preco_unitario/100.0f),
        item->quantidade,
        ((float)item->preco_total/100.0f));

    return NF_ERR_OK;
}
