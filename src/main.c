#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/list.h"
#include "../include/utils.h"
#include "../include/notafiscal.h"
#include "../include/operacoes_item.h"
#include "../include/operacoes_clientes.h"

int main(void) {
    List *lista_nfs = calloc(1, sizeof(*lista_nfs));
    if (lista_nfs == NULL) {
        printf("Não foi possível iniciar o programa. Memória insuficiente.\n");
        return EXIT_FAILURE;
    }
    list_init(lista_nfs, nfs_destroy);

    printf("====================Notas Fiscais====================\n");

    for (;;) {
        printf("Escolha uma opção:\n");
        printf("1. Imprimir Notas Fiscais \n2. Adicionar Nota Fiscal \n3. Adicionar item à uma nota fiscal \n4. Remover uma Nota Fiscal \n5. Remover um Item de uma Nota Fiscal \n0. Sair\n: ");
        int opcao = 0;
        if (pega_int(&opcao) != NF_ERR_OK) {
            fprintf(stderr, "Não foi possível pegar entrada de usuário devido erro crítico. O programa será encerrado.\n");
            goto cleanup;
        }
        printf("\n");

        switch (opcao) {
            case 0:
                break;

            case 1:
                 switch (imprimir_notas_fiscais(lista_nfs)) {
                     case NF_ERR_OK:
                        fprintf(stderr, "Todas as Notas Fiscais foram impressas.\n");
                        break;
                    case NF_ERR_SIZE_IS_ZERO:
                        printf("Nenhuma Nota Fiscal foi impressa, pois a lista está vazia.\n");
                        continue;
                    default:
                        fprintf(stderr, "Não foi possível imprimir lista de Notas Fiscais devido erro crítico. O programa será encerrado.\n");
                        goto cleanup;
                 }
                 continue;
            case 2: {
                NotaFiscal *nf = calloc(1, sizeof(*nf));
                if (nf == NULL) {
                    fprintf(stderr, "Não foi possível inicializar nota fiscal. O programa será encerrado.\n");
                }

                nf->itens = calloc(1, sizeof(*nf->itens));
                if (nf->itens == NULL) {
                    nfs_destroy(nf);
                    goto cleanup;
                }

                list_init(nf->itens, item_destroy);

                ClienteResult pega_cliente = cliente_pegar_dados();
                switch (pega_cliente.status) {
                    case NF_ERR_OK:
                        break;
                    case NF_ERR_NO_MEMORY_AVAILABLE:
                        fprintf(stderr, "Não foi possível possível pegar entrada de usuário. Memória insuficiente. O programa será encerrado.\n");
                        nfs_destroy(nf);
                        goto cleanup;
                    case NF_ERR_IO:
                        fprintf(stderr, "Não foi possível pegar entrada de usuário devido falha crítica. O programa será encerrado.\n");
                        nfs_destroy(nf);
                        goto cleanup;
                    default:
                        fprintf(stderr, "Falha crítica inesperada. O programa será encerrado.\n");
                        nfs_destroy(nf);
                        goto cleanup;
                }
                nf->cliente = pega_cliente.value;

                switch (list_ins_next(lista_nfs, lista_nfs->tail, nf)) {
                    case NF_ERR_OK:
                        break;
                    case NF_ERR_INVALID_ARG:
                        fprintf(stderr, "Não foi possível inserir o cliente à lista de notas fiscais. O programa será encerrado.\n");
                        nfs_destroy(nf);
                        goto cleanup;
                    case NF_ERR_NO_MEMORY_AVAILABLE:
                        fprintf(stderr, "Não foi possível armazenar cliente na lista de notas fiscais. O programa será encerrado.\n");
                        nfs_destroy(nf);
                        goto cleanup;
                    default:
                        fprintf(stderr, "Falha crítica inesperada. O programa será encerrado.\n");
                        nfs_destroy(nf);
                        goto cleanup;
                }

                printf("Cliente foi inserido à lista. Adicione um item à lista dele com a opção 3.\n");

                continue;
            }
            case 3: {
                printf("Insira o CPF da pessoa que deseja adicionar um item: ");
                StringoResult cliente_cpf = stringo_get_input();
                switch (cliente_cpf.status) {
                    case NF_ERR_OK:
                        break;
                    case NF_ERR_NO_MEMORY_AVAILABLE:
                        fprintf(stderr, "Não foi possível armazenar entrada de usuário. O programa será encerrado.\n");
                        goto cleanup;
                    case NF_ERR_IO:
                        fprintf(stderr, "Não foi possível pegar entrada de usuário. O programa será encerrado.\n");
                        goto cleanup;
                    default:
                        fprintf(stderr, "Falha crítica inesperada. O programa será encerrado.\n");
                        goto cleanup;
                }

                NotaFiscalResult nf_cliente = cliente_procurar(lista_nfs, cliente_cpf.value);
                switch (nf_cliente.status) {
                    case NF_ERR_OK:
                        break;
                    case NF_ERR_GENERIC_FAIL:
                        printf("Cliente não encontrado. Tente novamente.\n");
                        stringo_destroy(cliente_cpf.value);
                        continue;
                    case NF_ERR_INVALID_ARG:
                        fprintf(stderr, "Não foi possível encontrar cliente devido erro crítico. O programa será encerrado.\n");
                        stringo_destroy(cliente_cpf.value);
                        goto cleanup;
                    case NF_ERR_SIZE_IS_ZERO:
                        printf("A lista de clientes está vazia. Tente outra ação.\n");
                        stringo_destroy(cliente_cpf.value);
                        continue;
                    default:
                        fprintf(stderr, "Falha crítica inesperada. O programa será encerrado.\n");
                        stringo_destroy(cliente_cpf.value);
                        goto cleanup;
                }

                ItemResult item_result = item_pegar_dados();
                switch (item_result.status) {
                    case NF_ERR_OK:
                        break;
                    case NF_ERR_NO_MEMORY_AVAILABLE:
                        stringo_destroy(cliente_cpf.value);
                        fprintf(stderr, "Não foi possível armazenar dados do item. O programa será encerrado.\n");
                        goto cleanup;
                    case NF_ERR_INVALID_ARG:
                        stringo_destroy(cliente_cpf.value);
                        fprintf(stderr, "Não foi possível pegar dados do item devido falha crítica. O programa será encerrado.\n");
                        goto cleanup;
                    case NF_ERR_IO:
                        stringo_destroy(cliente_cpf.value);
                        fprintf(stderr, "Não foi possível pegar dados do item devido falha crítica. O programa será encerrado.\n");
                        goto cleanup;
                    default:
                        stringo_destroy(cliente_cpf.value);
                        fprintf(stderr, "Falha crítica inesperada. O programa será encerrado.\n");
                        goto cleanup;
                }

                NFError item_add_result = cliente_add_item(nf_cliente.value, item_result.value);
                switch (item_add_result) {
                    case NF_ERR_OK:
                        break;
                    case NF_ERR_INVALID_ARG:
                        fprintf(stderr, "Não foi possível inserir item à lista do cliente. O programa será encerrado.\n");
                        stringo_destroy(cliente_cpf.value);
                        item_destroy(item_result.value);
                        goto cleanup;
                    case NF_ERR_NO_MEMORY_AVAILABLE:
                        fprintf(stderr, "Não foi possível armazenar item na lista. O programa será encerrado.\n");
                        stringo_destroy(cliente_cpf.value);
                        item_destroy(item_result.value);
                        goto cleanup;
                    default:
                        fprintf(stderr, "Falha crítica inesperada. O programa será encerrado.\n");
                        stringo_destroy(cliente_cpf.value);
                        item_destroy(item_result.value);
                        goto cleanup;
                }

                stringo_destroy(cliente_cpf.value);
                printf("O item foi adicionado a lista do cliente.\n");
                continue;
            }
            default:
                printf("Opção não disponível no momento. Tente novamente.\n");
                continue;
        }

        break;
    }

    if (lista_nfs != NULL) {
        list_destroy(lista_nfs);
        free(lista_nfs);
    }
    return EXIT_SUCCESS;

cleanup:
if (lista_nfs != NULL) {
    list_destroy(lista_nfs);
    free(lista_nfs);
}
return EXIT_FAILURE;
}
