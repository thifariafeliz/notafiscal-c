Write a .md file containing instructions for me to improve my error handling in C language based on the current project. Don't be euphemist, don't be afraid to offend me. Judge me like you are a very wise Senior Developer in a very important company. Don't change any code.  

I’ll inspect the project’s C code and existing documentation, then write a candid, project-specific Markdown review focused strictly on error handling. I won’t modify source code.
```
• Ran pwd && rg --files -g 'AGENTS.md' -g '*.c' -g '*.h' -g 'Makefile' -g '*.md' -g '!b…
  └ include/operacoes_clientes.h
    include/list.h
    include/notafiscal.h
• Explored
  └ Read errors.h, notafiscal.h, list.h, stringo.h, utils.h, operacoes_clientes.h,
         operacoes_item.h, main.c, notafiscal.c, operacoes_item.c, utils.c, list.c,
         stringo.c, CORRECAO_ADDRESSSANITIZER.md, operacoes_clientes.c, Makefile, NEXT.…
```
The project already has several error enums and result structs, but their contracts are inconsistent. I’m tracing every failure path now—allocation, input, propagation, cleanup, and exit status—so the document distinguishes cosmetic issues from bugs that can leak memory, dereference null pointers, or misreport success.
```
• Ran git status --short
  └  M src/operacoes_clientes.c
     M src/operacoes_item.c
    ?? obj/operacoes_item.o
• Ran make -n debug=1
  └ gcc -std=c23 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wsign-conversi…
    gcc -std=c23 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wsign-conversi…
    gcc -std=c23 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wsign-conversi…
• Explored
  └ Read stringo.c, utils.c
    Search return |malloc|calloc|realloc|free|destroy|status|error|printf|fprintf|goto|
           list_init|list_ins|list_remove|ler_campo|take_int in src
• Failed (exit 1) gcc -std=c23 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -…
  └     1 | #include <cassert>
          |          ^~~~~~~~~
    compilation terminated.
```
Two facts set the tone for the review: the current source does not pass a syntax-only build (`DC_GENERIC_FAIL` is undefined, and C code includes the C++ header `<cassert>`), and several cleanup paths are themselves unsafe. The guide will prioritize restoring trustworthy contracts before adding any more error enums or messages.
```
• Added ERROR_HANDLING_REVIEW.md (+420 -0)
+# Error Handling Review: `notafiscal`
+
+## Senior verdict
• Ran git status --short && wc -l ERROR_HANDLING_REVIEW.md && sed -n '1,45p' ERROR_HAND…
  └ That would be unacceptable in a serious company because failure paths are not secon…
    
    Your next step is not to invent another enum. Make ownership explicit, make cleanup…
```
Created ERROR\_HANDLING\_REVIEW.md.

It contains a blunt, project-specific senior review covering:

- Critical memory and cleanup defects
- Dangling pointers and ownership failures
- Broken error propagation
- Misleading success messages and exit codes
- Input-handling problems
- A recommended error model
- A prioritized remediation plan
- Required failure-path tests
- A future review checklist

No source code was changed. I also confirmed the current source fails syntax checking due to `DC_GENERIC_FAIL` and the C++-only `<cassert>` header; both are documented.
```
  Worked for 2m 56s • 6:54 PM
```

===================================================================================================

How do I know an error is recoverable from? How can I properly recover from an error that is not fatal?  

An error is recoverable only when the program can take a specific action and continue in a known-valid state.

“Recoverable” is not an intrinsic property of an error code. It depends on where the error reaches the caller and what state the failed operation left behind.

## The recovery test

Before continuing after an error, answer these questions:

1. Is every data structure still valid?
2. Have all partially allocated resources been released?
3. Is ownership of every pointer known?
4. Did the failed operation avoid partially committing changes?
5. Can the caller do something meaningfully different next?
6. Will retrying have a realistic chance of succeeding?

If any of the first four answers is “no” or “I don’t know,” continuing is unsafe. That is not recovery; it is gambling with corrupted state.

If retrying performs exactly the same action under the same conditions, it is usually not recovery either.

## Classification for your project

| Error | Usually recoverable? | Correct response |
|---|---:|---|
| Empty or invalid integer | Yes | Explain the constraint and read a new complete line |
| Integer overflow | Yes | Tell the user the accepted range and retry |
| Client not found | Yes | Return to the menu or request another CPF |
| Duplicate or invalid business data | Yes | Reject the operation without modifying stored data |
| EOF | Usually not by retrying | Cancel the operation or terminate cleanly |
| Input stream I/O failure | Usually fatal | Clean up and exit with failure |
| Allocation failure | Usually fatal in this program | Roll back the current operation, clean up, and exit |
| Null argument inside internal code | No | Programming defect; report internal failure and exit |
| Broken list invariant | No | State may be corrupt; do not continue |
| Unknown status value | No | Treat as an internal programming error |

### Important distinctions

Invalid user input is recoverable because the program can obtain different input.

A missing client is recoverable because the database/list remains unchanged and the user can choose another client.

EOF is not recoverable by calling the input function again. EOF normally remains EOF. Your current pattern of printing “tente novamente” after `SE_EARLY_EOF` is therefore wrong. Treat EOF as cancellation or termination.

A failed `malloc` could theoretically be recoverable in a sophisticated system that can release caches or reject only one request. Your small interactive program has no such strategy. Returning to the menu and immediately allocating again is wishful thinking. Clean up and terminate with `EXIT_FAILURE`.

`LE_ARG_IS_NULL` indicates a caller bug, not something an end user can fix. Continuing after it could hide memory corruption or another broken invariant.

## Recovery requires atomic operations

A recoverable operation should behave transactionally:

```
Before operation: valid state A

Try operation:
    allocate temporary resources
    validate all input
    prepare the complete object

If anything fails:
    destroy temporary resources
    leave state A unchanged

If everything succeeds:
    commit once
    state becomes valid state B
```

For example, adding a fiscal note should not insert half a note and then ask whether recovery is possible. Construct the complete `NotaFiscal` privately. Only insert it into `caixa->notas_fiscais` after every required field succeeds.

If insertion fails, destroy the complete temporary note. The original `Caixa` remains unchanged, so the operation can safely return an error.

## Recover at the appropriate layer

Each layer should have one responsibility:

- `parse_int` detects invalid syntax or overflow.
- The input function reads a complete line and reports EOF or I/O failure.
- The note/client constructor cleans up partial objects.
- The menu decides whether to retry, return to the menu, or terminate.
- Only the presentation layer prints user-facing messages.

A parser should not decide to retry. It does not control input.

A constructor should not print “try again.” It does not control the interface.

A menu should not manually free the internal pieces of an object. It should call the object’s destructor.

## A practical control-flow pattern

Conceptually, your menu should handle errors like this:

```c
switch (status) {
    case NF_OK:
        printf("Item adicionado.\n");
        break;

    case NF_ERR_INVALID_INPUT:
    case NF_ERR_OUT_OF_RANGE:
        fprintf(stderr, "Valor inválido. Tente novamente.\n");
        /* Retry the relevant field or operation. */
        break;

    case NF_ERR_NOT_FOUND:
        fprintf(stderr, "Cliente não encontrado.\n");
        /* Return to the menu or request another CPF. */
        break;

    case NF_ERR_END_OF_INPUT:
        /* Treat EOF as cancellation or clean termination. */
        exit_status = EXIT_SUCCESS;
        goto cleanup;

    case NF_ERR_OUT_OF_MEMORY:
    case NF_ERR_IO:
        fprintf(stderr, "Não foi possível continuar.\n");
        exit_status = EXIT_FAILURE;
        goto cleanup;

    case NF_ERR_INVALID_ARGUMENT:
    case NF_ERR_CORRUPT_STATE:
    default:
        fprintf(stderr, "Erro interno do programa.\n");
        exit_status = EXIT_FAILURE;
        goto cleanup;
}
```

The exact names are less important than the policy. Every status must lead to an explicit action.

## Retry at the smallest useful scope

If only the quantity is invalid, request the quantity again. Do not discard the already valid description and restart the entire application.

However, only retry if the relevant state remains valid. A good field-reading loop looks conceptually like:

```c
for (;;) {
    status = read_quantity(&quantity);

    if (status == NF_OK) {
        break;
    }

    if (status == NF_ERR_INVALID_INPUT ||
        status == NF_ERR_OUT_OF_RANGE) {
        report_invalid_quantity();
        continue;
    }

    return status;
}
```

This works because malformed input does not corrupt anything and another line may contain a valid quantity.

Do not use the same loop for EOF, allocation failure, null arguments, or corrupted state.

## Your most important invariant

Every operation should guarantee:

```bash
success => requested change was committed completely
failure => requested change was not committed at all
```

That guarantee is what makes nonfatal recovery possible.

Right now, parts of your project leak partial objects and return dangling pointers. Until those ownership problems are fixed, many errors cannot be safely recovered from even if their enum names suggest otherwise. Error recovery starts with preserving valid state—not with printing “Tente novamente.”
