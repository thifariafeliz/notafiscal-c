#include <stdio.h>
#include <stdlib.h>

#include "../include/list.h"
#include "../include/utils.h"
#include "../include/notafiscal.h"
#include "../include/operacoes_item.h"
#include "../include/operacoes_clientes.h"

int main(void) {
    Caixa *caixa = malloc(sizeof(*caixa));
    if (caixa == NULL) {
        printf("Não foi possível alocar memória para `List *notafiscal`.\nO Programa está sendo encerrado.\n");
        return -1;
    }

    caixa->notas_fiscais = malloc(sizeof(*caixa->notas_fiscais));
    if (caixa->notas_fiscais == NULL) {
        printf("Não foi possível alocar espaço para notas fiscais.\n");
        free(caixa);
        return -1;
    }

    // list_init(caixa->notas_fiscais, nota_fiscal_destroy);
    list_init(caixa->notas_fiscais, dados_cliente_destroy);

    while (1) {
        printf("Escolha o que fazer:\n");
        printf("1. Ver clientes\n2. Adicionar Cliente\n3. Imprimir notas fiscais de um cliente\n\
4. Adicionar nota fiscal a um cliente\n5. Remover Cliente\n0. Sair\n: ");

        IntParseResult opcao;

        opcao = take_int();
        // buffer_flush();

        /// Aqui verificamos se a opção do usuário foi pega corretamente, só seguindo para o próximo bloco de código
        /// caso tudo tenha dado certo.
        switch (opcao.status) {
            case PARSE_OK:
                break;

            case PARSE_EMPTY:
            case PARSE_INVALID:
                printf("Valor inserido é inválido como número inteiro. Tente novamente.\n");
                continue;

            case PARSE_OVERFLOW:
                printf("Número muito pequeno ou muito grande para ser um número inteiro válido. Tente novamente.\n");
                continue;

            default:
                printf("Erro fatal. O programa será encerrado.\n");
                goto clearall;
        }

        /// O programa só chega até aqui se a opção escolhida pelo usuário for válida, e portanto, itera sobre
        /// as possíveis opções do menu.
        switch (opcao.value) {
            case 0:
                printf("O programa será encerrado. Até mais!\n");
                goto clearall;

            case 1:
                printf("\n");
                int print_result = imprimir_clientes(caixa);

                switch (print_result) {
                    case 0:
                        break;
                    case -1:
                        printf("Falha lógica. Caixa é nulo.\n");
                        goto clearall;
                    case -2:
                        printf("Não foi possível imprimir a lista, pois seu tamanho é zero.\n");
                        continue;
                    case -3:
                        printf("Falha lógica. Primeiro item da lista é nulo.\n");
                        goto clearall;
                }

                printf("\nTodos os clientes foram impressos.\n\n");
                break;

            case 2:
                printf("\n");
                DadosClienteResult cliente_result = pega_dados_cliente();
                switch (cliente_result.status) {
                    case SE_OK:
                        break;
                    case SE_EARLY_EOF:
                        printf("Ocorreu um erro ao pegar entrada de usuário. Early EOF. Tente novamente.\n");
                        buffer_flush();
                        continue;
                    case SE_FAILED_MALLOC:
                        printf("Não foi possível alocar espaço para dados do cliente.\nO programa será encerrado.\n");
                        goto clearall;
                    case SE_ARG_IS_NULL:
                        printf("Erro ao pegar entrada de usuário. Falha lógica.\n");
                        goto clearall;
                    default:
                        printf("Erro ao pegar entrada de usuário. Falha lógica desconhecida.\n");
                        goto clearall;
                }

                OpDadosItemResult item_result = pega_dados_item();

                switch (item_result.status) {
                    case DI_OK:
                        break;
                    case DI_STRINGO_ERROR:
                        printf("Ocorreu um erro ao pegar entrada de usuário.\n");
                        goto clearall;
                        case DI_FAILED_MALLOC:
                        printf("Não foi possível alocar espaço para dados do item.\nO programa será encerrado.\n");
                        goto clearall;
                    case DI_INT_ERROR:
                        printf("Erro ao pegar entrada de usuário. Falha lógica.\n");
                        goto clearall;
                    default:
                        printf("Erro ao pegar entrada de usuário. Falha lógica desconhecida.\n");
                        goto clearall;
                }

                // switch (list_ins_prev(caixa->notas_fiscais, caixa->notas_fiscais->head, (void*)result.cliente))
                // {
                //     case LE_OK:
                //         printf("Dados adicionados com sucesso à lista de notas fiscais.\n");
                //         break;
                //     case LE_ARG_IS_NULL:
                //         printf("Erro ao adicionar dados do cliente à lista. Falha lógica.\nO programa será encerrado.\n");
                //         goto clearall;
                //     case LE_FAILED_MALLOC:
                //         printf("Não há espaço para adicionar dados à lista.\nO programa será encerrado.\n");
                //         goto clearall;
                //     default:
                //         printf("Erro desconhecido ao adicionar dados à lista.\nO programa será encerrado.\n");
                //         goto clearall;
                // }
        }
    }

    return 0;

    /// Dá free em tudo que é necessário de acordo com o grau que atingiu durante o código
clearall:
    list_destroy(caixa->notas_fiscais);
    free(caixa->notas_fiscais);
    free(caixa);
    return 0;
}
