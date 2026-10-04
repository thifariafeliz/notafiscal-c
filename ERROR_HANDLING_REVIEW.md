> # [!note] Prompt original
> Write a .md file containing instructions for me to improve my error handling in C language based on the current project. Don't be euphemist, don't be afraid to offend me. Judge me like you are a very wise Senior Developer in a very important company. Don't change any code.

# Novo review do error handling: `notafiscal`

Revisão refeita em 04/10/2026 sobre o working tree atual. Este documento avalia o código que existe agora, não o estado descrito no review anterior. Nenhum arquivo-fonte foi alterado durante a revisão.

## Veredito de Senior

Você melhorou bastante a base, mas eu ainda rejeitaria este change request.

Agora existe um modelo único de status, os resultados com ponteiro respeitam em geral a regra “falhou, então `value == NULL`”, o projeto compila com a política severa de warnings e vários caminhos de cleanup ficaram muito melhores. Isso é progresso real, não cosmético.

Mas o programa ainda pode:

- imprimir que uma alocação falhou e imediatamente dereferenciar o ponteiro nulo;
- vazar um `Cliente` em qualquer falha durante sua construção;
- entrar em loop infinito ao tentar destruir uma lista inconsistente;
- dereferenciar um parâmetro de saída nulo em `pega_int`;
- provocar undefined behavior ao calcular o preço total;
- encontrar erro ao imprimir e mesmo assim retornar sucesso;
- corromper uma lista ao receber um nó pertencente a outra lista.

Em uma empresa séria, “compila limpo” é o piso, não o teto. O seu happy path está razoável; seus caminhos de falha ainda não são confiáveis. Error handling que falha justamente quando falta memória ou um invariante quebra não é error handling pronto.

## O que foi realmente corrigido desde o review anterior

É importante reconhecer exatamente o que mudou:

- O projeto agora compila como C; o include C++ inválido desapareceu.
- O identificador inexistente e o protocolo de tags numéricas de `AddItemClienteResult` desapareceram.
- Há um único `NFError` compartilhado e structs de resultado consistentes.
- `cliente_pegar_dados`, `item_pegar_dados` e `stringo_get_input` retornam `value == NULL` quando falham.
- `item_destroy` destrói a descrição e libera o próprio `Item`.
- `nfs_destroy` agora recebe e destrói uma `NotaFiscal`, em vez de tratá-la como `List`.
- `list_ins_prev` verifica o resultado de `malloc`.
- `cliente_procurar` finalmente avança para o próximo nó.
- O fluxo fatal de `main` chega a `EXIT_FAILURE`; a saída voluntária chega a `EXIT_SUCCESS`.
- A leitura de inteiros usa a mesma leitura dinâmica de linha que o restante do programa; o antigo “flush” cego sumiu.
- A transferência de ownership de uma nota ou item só ocorre depois que a inserção na lista tem sucesso; em falha de inserção, o chamador destrói o objeto.
- Os `switch` de `main` possuem `default`, então um status inesperado não cai mais silenciosamente numa mensagem de sucesso.

Essas correções atacam problemas centrais do review anterior. Você saiu de um modelo incoerente para uma base aproveitável. O problema agora é que alguns buracos restantes são fatais.

## Defeitos críticos

### 1. Você trata `calloc` falho e depois usa o ponteiro nulo

Em `src/main.c:49-54`:

```c
NotaFiscal *nf = calloc(1, sizeof(*nf));
if (nf == NULL) {
    fprintf(stderr, "Não foi possível inicializar nota fiscal. O programa será encerrado.\n");
}

nf->itens = calloc(1, sizeof(*nf->itens));
```

Falta encerrar o fluxo após o diagnóstico. Se a alocação falhar, `nf->itens` é uma dereferência de `NULL`. A mensagem promete que o programa será encerrado, mas o controle continua até undefined behavior.

Isso é exatamente o tipo de defeito que invalida a confiança no sistema inteiro: o caminho criado para lidar com falta de memória quebra por causa da própria falta de memória.

Correção exigida: toda ramificação de falha precisa alterar o controle de fluxo. Neste caso, depois da mensagem, vá para o cleanup fatal. Adote como regra de review: **um log de erro nunca substitui `return`, `goto`, `break` ou outra decisão explícita**.

### 2. Toda construção parcial de `Cliente` ainda vaza o objeto externo

`cliente_pegar_dados` aloca o `Cliente` em `src/operacoes_clientes.c:96`. Em cada falha posterior, chama `cliente_destroy(cliente)` e retorna. Porém `cliente_destroy`, em `src/operacoes_clientes.c:153-176`, destrói somente os quatro `Stringo`; ele nunca executa `free(cliente)`.

O vazamento ocorre em qualquer uma destas situações:

- EOF/erro ao ler CPF;
- EOF/erro ao ler nome depois de CPF já criado;
- falha ao ler endereço;
- falha ao ler telefone;
- qualquer falha de alocação dentro dessas leituras.

`nfs_destroy` tenta compensar esse contrato incompleto chamando `cliente_destroy` e depois `free(nf->cliente)`. Isso transforma “destruir cliente” em uma operação de duas etapas que somente um chamador conhece. O próprio construtor já esqueceu a segunda etapa.

Mantenha um destrutor canônico:

```text
cliente_destroy(cliente)
    destrói nome, CPF, endereço e telefone
    libera Cliente
```

Depois remova o `free(nf->cliente)` extra de `nfs_destroy`. Criação e destruição devem ser inversas exatas; nenhum chamador deveria saber que o “destroy” só destrói metade do objeto.

### 3. `pega_int` não valida seu parâmetro de saída

`src/utils.c:34-59` aceita `int *ret`, mas não verifica `ret == NULL`. Depois de ler e validar toda a entrada, executa `*ret = int_result.value` em `src/utils.c:54`.

A atribuição `ret = NULL` em `src/utils.c:45` não ajuda em nada:

- ela ocorre apenas numa ramificação que, com o código atual, é praticamente inalcançável, porque `input.value->data` veio de um `Stringo` válido;
- ela modifica somente a cópia local do ponteiro;
- ela não protege a dereferência na linha 54;
- ela não inicializa nem invalida o objeto do chamador.

O contrato correto é simples: valide `ret` antes de consumir qualquer entrada. Se for nulo, retorne `NF_ERR_INVALID_ARG` imediatamente. Em falha, não escreva no destino.

### 4. `list_destroy` ainda pode entrar em loop infinito

Em `src/list.c:28-32`, o loop continua enquanto `size > 0`. Se `list_remove` falhar, `size` não muda e o loop tenta a mesma operação eternamente.

Um estado fácil de demonstrar por inspeção é `size > 0` com `tail == NULL`: `list_remove` retorna `NF_ERR_INVALID_ARG`, e `list_destroy` gira para sempre. Como o destrutor é chamado justamente no cleanup fatal, uma corrupção anterior pode impedir até o encerramento do programa.

Você precisa escolher uma política explícita para lista corrompida. No mínimo:

- capture o retorno de `list_remove`;
- pare e propague a falha em vez de repetir para sempre;
- defina se, num erro parcial, os nós já removidos permanecem removidos;
- não execute `memset` e retorne sucesso se a destruição não terminou.

Destrutor não pode prometer sucesso após cleanup incompleto.

### 5. `list_remove` ainda pode dereferenciar `NULL` e corromper outra lista

`src/list.c:39-80` valida `list` e `node`, mas não valida `data`. A linha 52 executa `*data = node->data` incondicionalmente.

Além disso, `Node` contém `node->list`, mas `list_remove` nunca verifica se `node->list == list`. Um nó estrangeiro pode fazer a função:

- modificar os links da lista à qual ele realmente pertence;
- alterar `head` ou `tail` da lista recebida;
- decrementar o `size` da lista errada;
- liberar um nó ainda referenciado por outra estrutura.

O mesmo problema de pertencimento existe em `list_ins_next` e `list_ins_prev`: quando a lista não está vazia, o nó de referência precisa pertencer à lista recebida.

`NF_ERR_INVALID_NODE_LIST` já existe. Hoje ele quase não protege justamente as operações que mais precisam dele.

## Defeitos graves de propagação e honestidade do status

### 6. A impressão engole erros e retorna sucesso parcial

`imprimir_notas_fiscais` tem vários caminhos em que detecta uma inconsistência e mesmo assim termina com `NF_ERR_OK`:

- em `src/notafiscal.c:26-27`, uma nota nula é ignorada; pior, o nó não avança nesse `continue`;
- em `src/notafiscal.c:41-43`, uma cadeia de itens menor que `size` apenas interrompe o loop interno;
- em `src/notafiscal.c:45-47`, qualquer falha de `item_imprimir` apenas interrompe o loop interno;
- o retorno dos `printf` de separadores e títulos não é verificado.

`item_imprimir`, em `src/operacoes_item.c:67-82`, também retorna sucesso se `item->descricao->data == NULL` e ignora falhas dos seus dois `printf`.

Resultado: a opção 1 pode imprimir apenas uma parte dos dados, encontrar uma estrutura inválida ou uma falha de escrita e ainda fazer `main` anunciar “Todas as Notas Fiscais foram impressas.”

Isso é error handling desonesto. Se a operação promete imprimir tudo, qualquer falha relevante deve ser propagada. Se impressão parcial for aceitável, a API precisa ter um status específico e o chamador precisa dizer que o resultado foi parcial.

### 7. As validações de invariantes das listas são incompletas

`imprimir_notas_fiscais` considera vazia apenas a combinação `head == NULL && size == 0`. Se `size > 0 && head == NULL`, a linha 21 dereferencia `node == NULL`. Se `size == 0 && head != NULL`, o código retorna sucesso sem denunciar a estrutura inválida.

`cliente_add_item` detecta parte do estado inválido quando `size != 0`, mas aceita `size == 0` com `head` ou `tail` não nulos. `cliente_procurar` também confia que existem exatamente `size` nós; se a cadeia acabar antes, dereferencia `node_nf == NULL` na próxima iteração.

Não espalhe condições ligeiramente diferentes para representar “lista válida”. Defina invariantes únicos:

```text
size == 0  <=>  head == NULL && tail == NULL
size > 0   =>   head != NULL && tail != NULL
head->prev == NULL
tail->next == NULL
cada nó alcançável tem node->list == list
número de nós alcançáveis == size
```

Operações públicas devem validar o que podem receber do chamador. Invariantes internos impossíveis devem gerar `NF_ERR_CORRUPT_STATE` ou assertion em build de desenvolvimento — hoje esse status nem existe.

### 8. O cálculo de preço pode causar signed integer overflow

Em `src/operacoes_item.c:62`:

```c
item->preco_total = item->preco_unitario * item->quantidade;
```

Os dois valores aceitam qualquer `int`, inclusive negativos e extremos. Multiplicação com overflow de `int` tem undefined behavior em C. Seu parser detecta overflow textual, mas isso não torna segura uma operação aritmética posterior.

Valide domínio e multiplicação antes de calcular:

- preço unitário deve aceitar negativo?
- quantidade deve ser estritamente positiva?
- qual é o total máximo permitido?
- o produto cabe no tipo escolhido?

Para dinheiro, defina unidade e limites. Usar centavos inteiros é uma boa decisão, mas o tipo e as checagens precisam suportar o domínio. Um `int64_t` pode ser mais apropriado, ainda com validação de multiplicação.

### 9. Crescimento de `Stringo` não trata overflow de capacidade

Em `src/stringo.c:50-53`, `capacity *= 2` acontece sem checar `SIZE_MAX`. Quando a multiplicação transborda, o tamanho passado a `realloc` deixa de representar a necessidade real, e a escrita seguinte pode sair do buffer.

Esse caso é extremo, mas esse é precisamente o trabalho do error handling: falhar de modo definido antes de corromper memória. Verifique se a duplicação cabe antes de alterar `capacity`; só publique a nova capacidade depois que `realloc` tiver sucesso.

## O modelo de erro melhorou, mas ainda não é semântico

### `NF_ERR_GENERIC_FAIL` continua sendo uma gaveta de entulho

O comentário em `include/returnables.h:14` diz que `NF_ERR_GENERIC_FAIL` é usado no lugar de `false`. Ao mesmo tempo, `cliente_procurar` usa esse mesmo status para “cliente não encontrado”, enquanto `list_is_head` e `list_is_tail` o usam como resultado booleano falso.

Esses fatos exigem ações completamente diferentes. Um status de erro não deve também ser booleano e “não encontrado”.

Crie, no mínimo, `NF_ERR_NOT_FOUND` para busca. Para predicados como “é head?”, retorne `bool`; não force sucesso/falha a representar verdadeiro/falso. Reserve um erro genérico somente para uma causa que o chamador realmente não consegue classificar — idealmente, elimine-o.

### EOF e erro real de stream continuam colapsados

`stringo_get_input` trata todo `getchar() == EOF` sem dados como `NF_ERR_IO`. Ele não consulta `feof(stdin)` nem `ferror(stdin)`. EOF não é necessariamente falha de I/O: pode ser encerramento normal do pipe ou pedido de cancelamento pelo usuário.

Defina uma política:

- `NF_ERR_END_OF_INPUT`: entrada terminou antes do campo;
- `NF_ERR_IO`: o stream está com erro;
- sucesso para uma última linha não vazia sem `\n`, se essa for a decisão documentada.

Depois `main` decide se EOF no menu é saída limpa ou fatal. Hoje a camada de leitura remove essa escolha do chamador.

### Há categorias redundantes ou sem contrato

`NF_ERR_OVERFLOW` e `NF_ERR_OUT_OF_RANGE` coexistem sem diferença documentada. `NF_ERR_SIZE_IS_ZERO` às vezes representa uma situação normal de UI (“não há notas”) e às vezes um erro estrutural numa operação. `NF_ERR_INVALID_ARG` também é usado para estado interno inconsistente.

Organize o enum por ação do chamador, não por frase que parece descrever o problema:

- argumento inválido: bug do chamador;
- input inválido: informar e tentar novamente;
- fim de input: cancelar ou sair;
- I/O: falha fatal/diagnóstico;
- sem memória: cleanup e saída fatal;
- não encontrado: informar e voltar ao menu;
- estado corrompido: bug interno, cleanup limitado e saída fatal;
- overflow/range: mensagem específica e retry, quando veio do usuário.

## Ownership: o desenho que o código deve declarar

O grafo atual deveria ter este contrato:

```text
lista_nfs (List)
└── possui cada NotaFiscal
    ├── possui um Cliente
    │   ├── possui nome
    │   ├── possui CPF
    │   ├── possui endereço
    │   └── possui telefone
    └── possui a List de itens
        └── possui cada Item
            └── possui descrição
```

As transições de ownership precisam ser explícitas:

```text
cliente_pegar_dados -> chamador possui Cliente
atribuição nf->cliente -> NotaFiscal possui Cliente
list_ins_next(lista_nfs, ..., nf) com sucesso -> lista_nfs possui NotaFiscal

item_pegar_dados -> chamador possui Item
cliente_add_item com sucesso -> nf->itens possui Item
```

Em falha de inserção, ownership não muda. Essa parte está implementada corretamente em `main`; documente-a nos headers para que deixe de depender de leitura da implementação.

Todos os destruidores de objetos owning devem:

- aceitar `NULL`;
- destruir todos os filhos possuídos;
- liberar o próprio objeto;
- nunca exigir um `free` adicional do chamador.

Para containers alocados externamente, escolha nomes diferentes se o contrato for diferente, por exemplo “clear” para destruir conteúdo sem liberar o container e “destroy” para destruir tudo. Hoje `list_destroy` limpa o `List`, mas não libera o `List`, enquanto `item_destroy` libera o `Item` e `cliente_destroy` não libera o `Cliente`. A mesma palavra descreve três contratos diferentes.

## Diagnósticos e camada de UI

Houve melhora: a maior parte dos erros fatais em `main` usa `stderr`. Ainda há inconsistências:

- a falha inicial de `calloc` usa `printf` em `src/main.c:14`, portanto vai para `stdout`;
- a mensagem de sucesso da impressão usa `stderr` em `src/main.c:38`;
- funções de domínio e parsing, como `pega_int`, imprimem mensagens diretamente, misturando mecanismo com política de UI;
- mensagens diferentes colapsam causas diferentes em “falha crítica inesperada”.

Uma divisão saudável seria:

- `list.c`, `stringo.c`, parsing e operações de domínio retornam status e não imprimem diagnóstico;
- a camada de aplicação decide mensagem, retry, cancelamento e exit code;
- sucesso e conteúdo normal vão para `stdout`;
- diagnósticos de falha vão para `stderr`.

Se uma função retorna status, o chamador precisa poder agir com esse status. Se toda causa termina em “falha crítica inesperada”, o enum não está oferecendo informação suficiente.

## Contratos públicos ainda estão ausentes

Os headers declaram funções, mas não dizem:

- quais ponteiros podem ser `NULL`;
- se o retorno é borrowed ou owned;
- quando ownership é transferido;
- o estado do output em falha;
- quais statuses podem ser retornados;
- se a função imprime;
- se aceita objetos parcialmente inicializados;
- se `destroy` libera o próprio objeto ou apenas seus campos.

Isso já causou o contrato dividido de `cliente_destroy`. Comentários não são burocracia quando o compilador não consegue expressar ownership.

Formalize especialmente estes contratos:

```text
Result com ponteiro:
    status == NF_ERR_OK  => value != NULL e ownership definido
    status != NF_ERR_OK  => value == NULL

Inserção:
    sucesso => container assume ownership de data
    falha   => chamador mantém ownership de data

Busca:
    sucesso => value é borrowed; chamador não destrói
    falha   => value == NULL

Parâmetro de saída:
    argumento obrigatório não nulo
    falha não altera o destino
```

## Testes: atualmente não há evidência suficiente

Não existe suíte de testes no repositório. Para um trabalho focado em error handling, isso é uma lacuna séria. Falhas de alocação raramente aparecem em testes manuais; você precisa injetá-las.

Adicione testes para:

- falha de cada `malloc`, `calloc` e `realloc`, uma chamada por vez;
- `NotaFiscal` falhando na alocação do próprio objeto e depois na lista de itens;
- `Cliente` falhando antes e depois de cada campo adquirido;
- `Item` falhando antes e depois da descrição;
- falha ao inserir uma nota e um item já completamente construídos;
- destruição de objetos nulos, vazios, parciais e completos;
- `pega_int(NULL)` sem consumir input e sem crash;
- `list_remove` com `data == NULL`;
- remoção e inserção usando nó estrangeiro;
- lista inconsistente (`size`, `head`, `tail` e cadeia divergentes) sem loop infinito;
- busca do primeiro, meio, último e cliente ausente;
- EOF antes do campo e após uma última linha sem newline;
- `ferror` separado de EOF;
- inteiro vazio, inválido, limites exatos, underflow e overflow;
- quantidade/preço negativos e overflow do produto;
- falha de `printf`/stream de saída, provando que impressão parcial não vira sucesso;
- exit code zero somente em encerramento bem-sucedido.

Rode ASan, UBSan e LeakSanitizer/Valgrind em CI. Não dependa apenas de Valgrind manual: teste unitário com allocator injetável torna falha de memória determinística.

## Evidência desta revisão

O código atual foi compilado fora de `build/` e `obj/`, com todos os fontes e estas classes de verificação:

```text
-std=c23
-Wall -Wextra -Wpedantic -Werror
-Wshadow -Wconversion -Wsign-conversion
-Wformat=2 -Wnull-dereference
-fsanitize=address,undefined
```

Resultado: compilação e link concluíram sem warnings ou erros.

Também foram exercitados:

- entrada `0`: encerramento com status `0`;
- criação de nota interrompida por EOF: encerramento com status não zero;
- ambos sob ASan/UBSan, sem erro de acesso nesse percurso quando a detecção de leaks foi desabilitada.

LeakSanitizer não conseguiu operar neste ambiente por limitação de `ptrace`, e o Valgrind não iniciou por ausência dos símbolos necessários da libc. Portanto, não alego medição dinâmica de leaks. O vazamento de `Cliente` descrito acima é conclusão direta e determinística do fluxo de ownership: há um `calloc` do objeto, o cleanup libera apenas seus filhos, e nenhuma instrução libera o próprio objeto.

## Ordem de correção

### Fase 1 — impedir crash, hang e UB no caminho de erro

1. Encerre imediatamente o fluxo quando `calloc` de `NotaFiscal` falhar.
2. Valide `ret` no início de `pega_int`.
3. Faça `list_destroy` parar e propagar falha de remoção.
4. Valide `data` e pertencimento do nó nas operações de lista.
5. Valide overflow no preço total e no crescimento de `Stringo`.

Critério de saída: nenhum caminho de falha pode dereferenciar nulo, entrar em loop infinito, corromper lista ou executar signed overflow.

### Fase 2 — tornar destruição e ownership uniformes

1. Faça `cliente_destroy` liberar o objeto inteiro.
2. Ajuste `nfs_destroy` para não liberar `Cliente` duas vezes.
3. Diferencie claramente “clear” de “destroy” para containers.
4. Documente cada transferência de ownership nos headers.

Critério de saída: objetos nulos, parciais e completos são destruídos sem leak, double-free ou acesso inválido, inclusive sob falha injetada em cada alocação.

### Fase 3 — tornar status semanticamente honestos

1. Elimine o uso booleano de `NF_ERR_GENERIC_FAIL`.
2. Adicione statuses distintos para not found, end of input e estado corrompido.
3. Separe overflow/range apenas se exigirem ações diferentes.
4. Propague falhas de impressão; não retorne sucesso parcial silencioso.

Critério de saída: cada status público possui uma ação clara do chamador e um teste que força esse caminho.

### Fase 4 — separar mecanismo de UI e fechar com testes

1. Remova mensagens das camadas de estrutura, parsing e domínio.
2. Centralize decisões de retry, cancelamento e diagnóstico na aplicação.
3. Corrija o uso de `stdout` e `stderr`.
4. Automatize testes de allocator, stream, lista, constructors e exit status.

Critério de saída: a suíte prova os caminhos de falha sem depender de intervenção manual.

## Checklist obrigatório para o próximo review

- Toda falha altera o controle de fluxo, além de imprimir uma mensagem?
- Todo parâmetro obrigatório é validado antes de qualquer efeito colateral?
- Todo resultado com ponteiro retorna `NULL` em falha?
- Toda aquisição possui cleanup em cada falha posterior?
- Cada destrutor libera o objeto inteiro e é seguro com `NULL`?
- Ownership é transferido somente depois de sucesso confirmado?
- Todo nó é validado como membro da lista antes de mutação?
- Um estado inconsistente pode causar loop infinito no cleanup?
- Crescimentos e operações aritméticas checam overflow antes de executar?
- EOF, I/O, input inválido e not found continuam distinguíveis?
- Um erro de impressão pode virar sucesso?
- Todos os statuses têm ação e teste correspondentes?
- Falha fatal retorna `EXIT_FAILURE`?
- O teste força a falha ou apenas espera que ela aconteça?

## Julgamento final

Seu novo error handling é muito melhor que o anterior. A arquitetura não é mais uma coleção de enums incompatíveis e ponteiros pendurados; existe agora uma direção coerente.

Mas ainda não está pronto. O defeito de `calloc` em `main`, o vazamento sistemático de `Cliente`, o loop infinito possível no destrutor e a mutação de nós estrangeiros são problemas de severidade alta. Não são detalhes de estilo. Eles mostram que os contratos ainda vivem parcialmente na sua cabeça, e não integralmente nas APIs e nos testes.

Pare de adicionar mensagens e statuses por enquanto. Feche os contratos de ownership, faça cleanup falhar de forma definida, proteja todas as operações que podem causar UB e escreva testes que provoquem cada falha. Quando o programa conseguir falhar de maneira previsível, sem crash, leak, hang, corrupção ou mentira no status, aí o error handling estará digno de aprovação.
