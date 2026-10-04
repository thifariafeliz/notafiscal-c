#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../include/list.h"
#include "../include/stringo.h"
#include "../include/operacoes_clientes.h"

// This receives a single NotaFiscal and prints the Cliente information
// only after checking for invalid pointers first
NFError cliente_imprimir(NotaFiscal *nf) {
    if (nf == NULL || nf->cliente == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    Cliente *cliente = nf->cliente;

    if (cliente->nome == NULL || cliente->nome->data == NULL ||
        cliente->cpf == NULL || cliente->cpf->data == NULL ||
        cliente->endereco == NULL || cliente->endereco->data == NULL ||
        cliente->fone == NULL || cliente->fone->data == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    int result = printf("Nome: %s - CPF: %s\nEndereço: %s - Telefone: %s\n",
        cliente->nome->data, cliente->cpf->data, cliente->endereco->data, cliente->fone->data);

    return result < 0 ? NF_ERR_IO : NF_ERR_OK;
}

// This function iterates the lista_nfs and compares the
// cpf with the cpf on the data on the node being iterated
// The lista_nfs is a doubly linked list that stores NotaFiscal
// NotaFiscal stores a Client and a dllist of Items
NotaFiscalResult cliente_procurar(List *lista_nfs, Stringo *cpf) {
    if (lista_nfs == NULL || cpf == NULL || cpf->data == NULL) {
        return (NotaFiscalResult){.value = NULL, .status = NF_ERR_INVALID_ARG};
    }

    if (lista_nfs->size == 0) {
        return (NotaFiscalResult){.value = NULL, .status = NF_ERR_SIZE_IS_ZERO};
    }

    Node *node_nf = lista_nfs->head;
    if (node_nf == NULL) {
        return (NotaFiscalResult){.value = NULL, .status = NF_ERR_INVALID_ARG};
    }

    for (size_t i = 0; i < lista_nfs->size; i++) {
        NotaFiscal *nf = node_nf->data;
        if (nf == NULL) {
            return (NotaFiscalResult){.value = NULL, .status = NF_ERR_INVALID_ARG};
        }

        Cliente *cliente = nf->cliente;
        if (cliente == NULL) {
            return (NotaFiscalResult){.value = NULL, .status = NF_ERR_INVALID_ARG};
        }

        if (strcmp(cliente->cpf->data, cpf->data) == 0) {
            return (NotaFiscalResult){.value = nf, .status = NF_ERR_OK};
        }

        node_nf = node_nf->next;
    }

    return (NotaFiscalResult){.value = NULL, .status = NF_ERR_GENERIC_FAIL};
}

// This function will add an Item to a Cliente
// Receives only a nota fiscalm, that contains the cliente and itens dllist, and an item
NFError cliente_add_item(NotaFiscal *nf, Item *item) {
    if (nf == NULL || nf->itens == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    if ((nf->itens->head == NULL || nf->itens->tail == NULL) && nf->itens->size != 0) {
        return NF_ERR_INVALID_ARG;
    }

    if (item == NULL || item->descricao == NULL || item->descricao->data == NULL) {
        return NF_ERR_INVALID_ARG;
    }

    NFError ins_result = list_ins_next(nf->itens, nf->itens->tail, item);

    if (ins_result != NF_ERR_OK) {
        return ins_result;
    }

    return NF_ERR_OK;
}

ClienteResult cliente_pegar_dados(void) {
    Cliente *cliente = calloc(1, sizeof(*cliente));
    if (cliente == NULL) {
        return (ClienteResult){.value = NULL, .status = NF_ERR_NO_MEMORY_AVAILABLE};
    }

    // Take Cliente's CPF input
    printf("Insira o CPF do cliente: ");
    StringoResult input_result = stringo_get_input();
    if (input_result.status != NF_ERR_OK) {
        if (input_result.status != NF_ERR_NO_MEMORY_AVAILABLE) {
            stringo_destroy(input_result.value);
        }
        cliente_destroy(cliente);
        return (ClienteResult){.value = NULL, .status = input_result.status};
    }
    cliente->cpf = input_result.value;

    // Take Cliente's Name input
    printf("Insira o nome do cliente: ");
    input_result = stringo_get_input();
    if (input_result.status != NF_ERR_OK) {
        if (input_result.status != NF_ERR_NO_MEMORY_AVAILABLE) {
            stringo_destroy(input_result.value);
        }
        cliente_destroy(cliente);
        return (ClienteResult){.value = NULL, .status = input_result.status};
    }
    cliente->nome = input_result.value;

    // Take Cliente's Address input
    printf("Insira o endereço do cliente: ");
    input_result = stringo_get_input();
    if (input_result.status != NF_ERR_OK) {
        if (input_result.status != NF_ERR_NO_MEMORY_AVAILABLE) {
            stringo_destroy(input_result.value);
        }
        cliente_destroy(cliente);
        return (ClienteResult){.value = NULL, .status = input_result.status};
    }
    cliente->endereco = input_result.value;

    // Take Cliente's Phone Number input
    printf("Insira o número de telefone do cliente: ");
    input_result = stringo_get_input();
    if (input_result.status != NF_ERR_OK) {
        if (input_result.status != NF_ERR_NO_MEMORY_AVAILABLE) {
            stringo_destroy(input_result.value);
        }
        cliente_destroy(cliente);
        return (ClienteResult){.value = NULL, .status = input_result.status};
    }
    cliente->fone = input_result.value;

    return (ClienteResult){.value = cliente, .status = NF_ERR_OK};
}

// This is the destructor function to the Cliente struct
void cliente_destroy(void *data) {
    if (data == NULL) {
        return;
    }

    Cliente *cliente = (Cliente*) data;

    if (cliente->nome != NULL) {
        stringo_destroy(cliente->nome);
    }

    if (cliente->cpf != NULL) {
        stringo_destroy(cliente->cpf);
    }

    if (cliente->endereco != NULL) {
        stringo_destroy(cliente->endereco);
    }

    if (cliente->fone != NULL) {
        stringo_destroy(cliente->fone);
    }

    return;
}
