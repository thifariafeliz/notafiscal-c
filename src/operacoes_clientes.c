#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../include/list.h"
#include "../include/stringo.h"
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

void dados_cliente_destroy(void *data) {
    DadosCliente *cliente = (DadosCliente*) data;

    if (cliente == NULL) {
        return;
    }

    if (cliente->nome != NULL) {
        stringo_destroy(cliente->nome);
    }

    if (cliente->cpf != NULL) {
        stringo_destroy(cliente->cpf);
    }

    if (cliente->fone != NULL) {
        stringo_destroy(cliente->fone);
    }

    if (cliente->endereco != NULL) {
        stringo_destroy(cliente->endereco);
    }

    free(cliente);
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



/*
  • A redução com menor trade-off seria manter a sequência explícita e
   extrair apenas as responsabilidades repetidas em src/
   operacoesClientes.c:40:

   - Crie uma função auxiliar ler_campo(prompt, destino) que:
       - exiba o prompt;
       - chame stringo_get_input();
       - trate/imprima o erro;
       - devolva somente o StringoError.

   - Crie stringo_destroy() e dados_cliente_destroy(). Assim, todo
     erro usa uma única rotina de limpeza, eliminando o grau e o
     grande switch final.

   - Aloque cliente com calloc. Os campos começam como NULL,
     permitindo que dados_cliente_destroy() libere somente o que já
     foi preenchido.

   - Atribua cada resultado diretamente ao respectivo campo após a
     leitura bem-sucedida. Atualmente as atribuições só acontecem no
     final, mas o bloco de erro tenta acessar cliente->nome,
     cliente->cpf etc. ainda não inicializados.

   - Preserve o erro original no retorno. Hoje qualquer falha acaba
     retornando SE_FAILED_MALLOC, mesmo quando ocorreu SE_EARLY_EOF
     ou outro erro.

   - Evite inicialmente transformar tudo em vetor de prompts e
     ponteiros. Isso diminuiria mais linhas, mas adicionaria
     indireção e dificultaria um pouco a leitura. Depois dos helpers,
     a função já ficará pequena e linear.

   - Corrija também o switch de limpeza: o case 3 cai no case 4,
     podendo causar liberações duplicadas, e alguns caminhos chegam
     ao fim da função sem return.

   - Como ajuste pequeno, declare a função como
     pega_dados_cliente(void) em vez de pega_dados_cliente() e
     corrija "telefneo" para "telefone".

   O formato final ideal seria: alocar → ler quatro campos com quatro
   verificações simples → retornar sucesso; em qualquer erro, chamar
   uma única função de destruição e retornar o status recebido.
 */
