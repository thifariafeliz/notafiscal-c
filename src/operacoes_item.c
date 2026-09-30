#include <stdio.h>
#include <stdlib.h>

#include "../include/utils.h"
#include "../include/operacoes_item.h"

OpDadosItemResult pega_dados_item(void) {
    DadosItem *item = calloc(1, sizeof(*item));
    if (item == NULL) {
        return (OpDadosItemResult){.item = item, .status = DI_FAILED_MALLOC};
    }

    StringoError result = ler_campo("Insira a descrição do item:", &item->descricao);
    if (result != SE_OK) {
        dados_item_destroy(item);
        return (OpDadosItemResult){.item = item, .status = DI_STRINGO_ERROR};
    }

    buffer_flush();
    printf("Insira o código do item: ");
    IntParseResult int_result = take_int();
    if (int_result.status != PARSE_OK) {
        dados_item_destroy(item);
        return (OpDadosItemResult){.item = item, .status = DI_INT_ERROR};
    }
    item->codigo = int_result.value;

    buffer_flush();
    printf("Insira a quantidade do item: ");
    int_result = take_int();
    if (int_result.status != PARSE_OK) {
        dados_item_destroy(item);
        return (OpDadosItemResult){.item = item, .status = DI_INT_ERROR};
    }
    item->quantidade = int_result.value;

    buffer_flush();
    printf("Insira o preço unitário do item: ");
    int_result = take_int();
    if (int_result.status != PARSE_OK) {
        dados_item_destroy(item);
        return (OpDadosItemResult){.item = item, .status = DI_INT_ERROR};
    }
    item->preco_unitario = int_result.value;

    buffer_flush();
    printf("Insira o preço total do item: ");
    int_result = take_int();
    if (int_result.status != PARSE_OK) {
        dados_item_destroy(item);
        return (OpDadosItemResult){.item = item, .status = DI_INT_ERROR};
    }
    item->preco_total = int_result.value;

    return (OpDadosItemResult){.item = item, .status = DI_OK};
}

void dados_item_destroy(void *data) {
    if (data == NULL) {
        return;
    }

    DadosItem *item = (DadosItem*) data;
    if (item->descricao == NULL) {
        return;
    }

    stringo_destroy(item->descricao);

    return;
}
