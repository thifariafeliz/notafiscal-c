# Feito no dia 29/09/2026
Apaguei praticamente tudo, já que o Codex disse que meu error handling estava um porcaria :).
Transformaei o `./include/errors.h` em `./include/returnables.h`. Isso é melhor, pois não são apenas erros lá. E os enums de erro estavam muitos, simplifiquei em um único enum que por enquanto está funcionando como `status` em structs de `value and status`.
Fiz a função `./src/operacoes_clientes.c/procurar_cliente()` itera sobre os clientes e retorna o que combina com o cpf informado previamente nos argumentos.

# Para se fazer:
- Implementar  as outras funções do arquivo `./include/operacoes_clientes.c`
