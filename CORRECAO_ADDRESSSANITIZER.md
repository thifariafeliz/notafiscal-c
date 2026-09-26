# Como corrigir o `AddressSanitizer: DEADLYSIGNAL`

## Resumo do problema

O erro não está sendo causado pelo `free` em si. A lista está guardando um tipo de dado e tentando destruí-lo como se fosse outro tipo.

Em `src/main.c`, a lista é configurada com o destrutor de uma nota fiscal:

```c
list_init(caixa->notas_fiscais, nota_fiscal_destroy);
```

Entretanto, na opção 2 do menu, o objeto inserido nessa mesma lista é um `DadosCliente *`:

```c
DadosClienteResult result = pega_dados_cliente();

list_ins_prev(
    caixa->notas_fiscais,
    caixa->notas_fiscais->head,
    result.cliente
);
```

Quando o programa termina, `list_destroy()` chama `nota_fiscal_destroy()` para esse objeto. A função faz o seguinte cast:

```c
NotaFiscal *nf = data;
```

O cast não converte um cliente em nota fiscal. Ele apenas manda o compilador interpretar os mesmos bytes com outro formato. Em seguida, esta linha acessa um campo que não existe no objeto realmente armazenado:

```c
dados_cliente_destroy(nf->cliente);
```

Por causa da disposição atual dos campos na memória, `nf->cliente` acaba lendo o ponteiro `endereco` do `DadosCliente` original. Depois, esse objeto `Stringo` é interpretado incorretamente como outro `DadosCliente`. Finalmente, `stringo_destroy()` recebe um endereço inválido e tenta lê-lo/liberá-lo.

Isso explica a pilha informada pelo AddressSanitizer:

```text
stringo_destroy
dados_cliente_destroy
nota_fiscal_destroy
list_destroy
main
```

O valor `0xbebebebe...` também é uma pista: o sanitizador usa esse padrão para tornar visível o acesso a memória não inicializada ou envenenada. Não se deve tentar contornar o problema testando esse valor; é preciso corrigir o tipo armazenado na lista.

## Correção recomendada para o estado atual do programa

A opção 2 adiciona clientes e `imprimir_clientes()` também interpreta cada nó como `DadosCliente *`. Portanto, no estado atual, o contrato real dessa lista é armazenar clientes. Seu destrutor também deve destruir clientes.

Como o callback da lista possui a assinatura `void (*)(void *)`, crie um pequeno adaptador em `src/main.c`, antes de `main()`:

```c
static void destruir_cliente_da_lista(void *data) {
    dados_cliente_destroy((DadosCliente *)data);
}
```

Depois, troque:

```c
list_init(caixa->notas_fiscais, nota_fiscal_destroy);
```

por:

```c
list_init(caixa->notas_fiscais, destruir_cliente_da_lista);
```

O adaptador evita passar uma função de tipo incompatível ao callback. Não faça apenas um cast no ponteiro da função, pois isso esconderia a incompatibilidade em vez de corrigi-la.

Também é recomendável renomear `Caixa::notas_fiscais` para `clientes`, já que esse é o conteúdo atual da lista. Esse renome não é necessário para eliminar o `SEGV`, mas reduz a chance de o mesmo erro reaparecer.

## Se a lista realmente deve armazenar notas fiscais

Nesse caso, mantenha `nota_fiscal_destroy` como callback, mas pare de inserir `DadosCliente *` diretamente. Cada nó deverá receber um `NotaFiscal *` válido, com seus campos inicializados. Todas as funções que percorrem a lista também deverão acessar primeiro a nota e, depois, seu cliente:

```c
NotaFiscal *nf = iterator->data;
DadosCliente *cliente = nf->cliente;
```

Não basta alterar apenas o cast de `imprimir_clientes()`: criação, inserção, leitura e destruição precisam concordar sobre um único tipo para os elementos da lista.

O contrato deve ser um destes:

```text
Lista de clientes:       Node::data -> DadosCliente
                        destrutor  -> dados_cliente_destroy (via adaptador)

Lista de notas fiscais: Node::data -> NotaFiscal
                        destrutor  -> nota_fiscal_destroy
```

Nunca misture os dois contratos na mesma lista.

## Ajustes adicionais importantes

### 1. Verificar a falha de `malloc` em `list_ins_prev`

Atualmente, `src/list.c` usa `new_node` imediatamente após a alocação. Inclua a verificação antes de acessar `new_node->data`:

```c
Node *new_node = malloc(sizeof(*new_node));
if (new_node == NULL) {
    return LE_FAILED_MALLOC;
}

new_node->data = data;
```

Sem isso, uma falha de alocação provoca outro `SEGV`.

### 2. Liberar o cliente se a inserção falhar

Depois que `pega_dados_cliente()` retorna com sucesso, `result.cliente` pertence ao chamador até ser inserido na lista. Se `list_ins_prev()` falhar, libere o cliente antes de sair:

```c
ListError status = list_ins_prev(
    caixa->notas_fiscais,
    caixa->notas_fiscais->head,
    result.cliente
);

if (status != LE_OK) {
    dados_cliente_destroy(result.cliente);
    /* tratar status e encerrar ou continuar */
}
```

Depois de uma inserção bem-sucedida, a lista passa a ser a dona do objeto e o libera em `list_destroy()`.

### 3. Não devolver um ponteiro pendente após um erro

Em `pega_dados_cliente()`, os caminhos de erro destroem `cliente`, mas ainda devolvem o antigo endereço no resultado:

```c
dados_cliente_destroy(cliente);
return (DadosClienteResult){.cliente = cliente, .status = result};
```

Troque esses retornos por:

```c
dados_cliente_destroy(cliente);
return (DadosClienteResult){.cliente = NULL, .status = result};
```

Isso evita o uso acidental de um ponteiro que já foi liberado.

### 4. Validar corretamente o destino em `ler_campo`

Antes de acessar `*str`, confirme que `str` não é nulo:

```c
StringoError ler_campo(char *prompt, Stringo **str) {
    if (prompt == NULL || str == NULL) {
        return SE_ARG_IS_NULL;
    }

    stringo_destroy(*str);
    *str = NULL;

    /* restante da função */
}
```

`stringo_destroy(NULL)` já é seguro. Definir `*str` como `NULL` depois da destruição também deixa o estado consistente se a nova leitura falhar.

## Como confirmar a correção

Faça uma compilação limpa com os sanitizadores habilitados:

```sh
make clean
make debug=1
```

Execute o programa e teste, nesta ordem:

1. Escolha a opção `2`.
2. Preencha nome, CPF, telefone e endereço.
3. Escolha a opção `1` e confira os dados impressos.
4. Escolha a opção `0` para forçar a destruição da lista.

Depois da correção, o programa deve encerrar sem `AddressSanitizer: DEADLYSIGNAL`, sem `heap-use-after-free`, sem `attempting free on address which was not malloc()-ed` e sem vazamentos relacionados ao cliente inserido.

## Regra para evitar esse erro

Uma lista genérica em C não sabe qual é o tipo de `Node::data`. Essa informação existe somente no contrato definido pelo programa. Portanto, para cada lista, mantenha sempre alinhados:

1. o tipo alocado;
2. o tipo inserido;
3. o tipo usado ao ler cada nó;
4. o destrutor registrado em `list_init()`.

Se qualquer um desses quatro pontos usar um tipo diferente, um cast para `void *` pode ocultar o erro durante a compilação, mas o acesso inválido aparecerá em tempo de execução.
