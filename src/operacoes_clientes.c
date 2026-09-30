#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../include/list.h"
#include "../include/stringo.h"
#include "../include/operacoes_item.h"
#include "../include/operacoes_clientes.h"

int imprimir_clientes(Caixa *caixa) {
    if (caixa == NULL) {
        return -1;
    }

    if (caixa->notas_fiscais->size == 0) {
        return -2;
    }

    Node *iterator = caixa->notas_fiscais->head;
    if (iterator == NULL) {
        return -3;
    }

    for (size_t i = 0; i < caixa->notas_fiscais->size; i++) {
        if (iterator == NULL) {
            return 0;
        }
        DadosCliente *cliente = (DadosCliente*) iterator->data;

        printf("%zu. %s, %s\n%s, %s\n", i, cliente->nome->data, cliente->cpf->data,
            cliente->fone->data, cliente->endereco->data);

        iterator = iterator->next;
    }

    printf("\n");

    return 0;
}

DadosClienteResult pega_dados_cliente(void) {
    DadosCliente *cliente = calloc(1, sizeof(*cliente));
    if (cliente == NULL) {
        return (DadosClienteResult){.cliente = cliente, .status = SE_FAILED_MALLOC};
    }

    StringoError result = ler_campo("Insira o nome do cliente", &cliente->nome);
    if (result != SE_OK) {
        dados_cliente_destroy(cliente);
        return (DadosClienteResult){.cliente = cliente, .status = result};
    }

    result = ler_campo("Insira o cpf do cliente", &cliente->cpf);
    if (result != SE_OK) {
        dados_cliente_destroy(cliente);
        return (DadosClienteResult){.cliente = cliente, .status = result};
    }

    result = ler_campo("Insira o telefone do cliente", &cliente->fone);
    if (result != SE_OK) {
        dados_cliente_destroy(cliente);
        return (DadosClienteResult){.cliente = cliente, .status = result};
    }

    result = ler_campo("Insira o endereço do cliente", &cliente->endereco);
    if (result != SE_OK) {
        dados_cliente_destroy(cliente);
        return (DadosClienteResult){.cliente = cliente, .status = result};
    }

    return (DadosClienteResult){.cliente = cliente, .status = SE_OK};
}

AddItemClienteResult adicionar_item_cliente(Caixa *caixa) {
    StringoReturnResult cpf_cliente = stringo_initialize();
    StringoError cpf_result = ler_campo("Insira o CPF do cliente desejado: ", &cpf_cliente.string);

    if (cpf_result != SE_OK) {
        return (AddItemClienteResult){.result_b = cpf_result, .result_a = 0, .result_c = 0, .error_type = 2};
    }

    NotaFiscal *cliente = procura_cliente(caixa, cpf_cliente.string);

    if (cliente == NULL) {
        return (AddItemClienteResult){.result_b = 0, .result_a = 0, .result_c = 0, .error_type = 4};
    }

    OpDadosItemResult item_result = pega_dados_item();

    if (item_result.status != DI_OK) {
        return (AddItemClienteResult){.result_a = item_result.status, .result_b = 0, .result_c = 0, .error_type = 1};
    }

    ListError list_ins_result = list_ins_next(cliente->item, cliente->item->tail, item_result.item);
    if (list_ins_result != LE_OK) {
        printf("Não foi possível inserir o item na nota fiscal do cliente.\n");
        return (AddItemClienteResult){.error_type = 3, .result_c = list_ins_result, .result_a = 0, .result_b = 0};
    }

    printf("O item foi inserido na nota fiscal do cliente.\n");

    return (AddItemClienteResult){.result_a = 0, .result_b = 0, .result_c = 0, .error_type = 0};
}

NotaFiscal *procura_cliente(Caixa *caixa, Stringo *cpf) {
    NotaFiscal *nota = NULL;

    List *lista_notas_fiscais = caixa->notas_fiscais;
    Node *node = lista_notas_fiscais->head;

    for (size_t i = 0; i < lista_notas_fiscais->size; i++) {
        NotaFiscal *nf = node->data;

        if (strcmp(nf->cliente->cpf->data, cpf->data) == 0) {
            nota = nf;
            break;
        }
    }

    return nota;
}
