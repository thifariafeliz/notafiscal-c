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
    list_init(caixa->notas_fiscais, nota_fiscal_destroy);

    while (1) {
        printf("Escolha o que fazer:\n");
        printf("1. Ver clientes\n2. Adicionar Cliente\n3. Imprimir notas fiscais de um cliente\n\
4. Adicionar item a uma nota fiscal\n5. Remover Cliente\n0. Sair\n: ");

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
                imprimir_notas_fiscais(caixa);

                printf("\nTodos os clientes foram impressos.\n\n");
                break;

            case 2:
                printf("\n");

                List *lista_de_itens = malloc(sizeof(*lista_de_itens));
                if (lista_de_itens == NULL) {
                    printf("Erro: não foi possível alocar espaço para lista de itens.\nl");
                    goto clearall;
                }

                list_init(lista_de_itens, dados_item_destroy);

                NotaFiscal *notinha = malloc(sizeof(*notinha));
                if (notinha == NULL) {
                    printf("Não há mais espaço para notas fiscais. O programa será encerrado.\n");
                    goto clearall;
                }

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

                notinha->cliente = cliente_result.cliente;
                notinha->item = lista_de_itens;



                if (list_ins_next(caixa->notas_fiscais, caixa->notas_fiscais->tail, notinha) != 0) {
                    printf("Não foi possível inserir nota fiscal na lista do caixa.\n");
                    goto clearall;
                }
                printf("A nota fiscal foi inserida na lista do caixa.\n");
                continue;

            case 3:
            printf("\n");
            StringoReturnResult cpf = stringo_initialize();
            StringoError result = ler_campo("Insira o cpf do cliente: ", &cpf.string);

            switch (result) {
                case SE_OK:
                    break;
                case SE_EARLY_EOF:
                    printf("Erro ao pegar entrada de usuário. Early EOF. Tente novamente.\n");
                    buffer_flush();
                    continue;
                case SE_FAILED_MALLOC:
                    printf("Não foi possível alocar memória para entrada de usuário.\n");
                    goto clearall;
                case SE_ARG_IS_NULL:
                    printf("Erro de falha lógica. O programa será encerrado.\n");
                case SE_GENERIC_FAIL:
                    printf("Erro de lógica desconhecido. O programa será encerrado.\n");
            }

            AddItemClienteResult add_result = adicionar_item_cliente(caixa);

            switch (add_result.error_type) {
                case 0:
                    break;
                case 4:
                    printf("Falha ao encontrar cliente. Tente novamente.\n");
                    continue;
                case 1:
                    printf("Falha lógica grave. Argumento para função era nulo. O programa será encerrado.\n");
                    goto clearall;

                case DC_GENERIC_FAIL:
                    printf("Falha lógica grave desconhecida. O programa será encerrado.\n");
                    goto clearall;
            }

            printf("O item foi adicionado com sucesso ao cliente.\n");

            continue;
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
