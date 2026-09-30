> # [!note] Prompt:
> Write a .md file containing instructions for me to improve my error handling in C language based on the current project. Don't be euphemist, don't be afraid to offend me. Judge me like you are a very wise Senior Developer in a very important company. Don't change any code.

# Error Handling Review: `notafiscal`

## Senior verdict

This error handling is not production-ready. In an important codebase, I would reject this change in review.

The problem is not that you forgot a few `if` statements. The problem is that your program has no consistent contract for failure, ownership, or cleanup. You created several enums and result structs, but the callers frequently ignore them, misinterpret them, discard the useful detail, or report success after failure. That is worse than having a simple error model, because the types create a false appearance of safety.

The current source does not even pass a syntax-only compilation with the project's warning policy:

- `src/main.c:152` uses the nonexistent `DC_GENERIC_FAIL`.
- `src/operacoes_clientes.c:1` includes the C++ header `<cassert>` in a C program.

Until the code builds cleanly, exits truthfully, and releases every partially constructed object, adding more features is irresponsible.

## What you are doing well

There is a useful foundation here:

- You check several allocation results.
- You use `strtol` instead of pretending that `atoi` can report errors.
- You use status-plus-value result structs.
- You use `calloc` for partially constructed compound objects.
- Some constructors attempt to clean up on failure.
- The Makefile enables strong warnings and sanitizers.

Those choices show that you know the relevant tools. The failure is in applying them consistently. Error handling is a whole-program design problem, not decoration around individual calls.

## The rule you must internalize

For every function that can fail, define these five things before writing its body:

1. What counts as success?
2. What distinct failures can the caller act on?
3. Who owns every input and allocation before, during, and after the call?
4. What state is every output in on failure?
5. Which layer turns the failure into a user-facing message?

If you cannot answer all five, the function's API is unfinished.

On failure, use one non-negotiable invariant for pointer-bearing results:

```c
status != OK  ==>  returned_pointer == NULL
status == OK  ==>  returned_pointer is valid and owned as documented
```

Your project violates this invariant repeatedly.

## Critical defects: fix these before anything else

### 1. Your destruction contracts are broken

`src/main.c:25` registers `nota_fiscal_destroy` as the destructor for a list of `NotaFiscal *`. However, `src/notafiscal.c:8` casts the received object to `List *` and calls `list_destroy` on it. A `NotaFiscal` is not a `List`. This is undefined behavior, not an imperfect cleanup strategy.

`nota_fiscal_destroy` must destroy exactly one `NotaFiscal` according to its ownership graph:

```text
NotaFiscal
├── cliente -> DadosCliente -> four Stringo objects
└── item    -> List -> Nodes -> DadosItem objects -> descricao Stringo
```

After destroying children, it must free the list object and the `NotaFiscal` itself. Creation and destruction must be exact inverses.

There is a second broken contract in `src/operacoes_item.c:58-70`: `dados_item_destroy` destroys `descricao` but never frees the `DadosItem`. Worse, if `descricao == NULL`, it returns immediately and leaks the item. This function is registered as a list destructor in `src/main.c:80`, so every item stored in that list leaks.

You also have two alleged item destructors, `dados_item_destroy` and `nf_item_destroy`, with different behavior and incompatible signatures. That is needless ambiguity around the most dangerous part of a C program. Keep one canonical destructor with the callback-compatible signature `void destroy(void *)`.

### 2. You return dangling pointers as results

In `pega_dados_cliente`, lines 77-96 free `cliente` and then return the freed address in `DadosClienteResult.cliente`.

In `pega_dados_item`, lines 15-51 destroy `item` and then return the stale address in `OpDadosItemResult.item`.

This is a trap for every caller. A failed result must carry `NULL`, never a historical address that is no longer valid. Setting a local pointer to `NULL` inside a destructor does not update the caller's pointer.

### 3. Partial construction leaks all over `main`

The option-2 path allocates, in order:

1. `lista_de_itens`;
2. `notinha`;
3. the client and its fields;
4. a list node during insertion.

Failure after any step must release everything acquired by earlier steps. The current code does not do that:

- If allocating `notinha` fails, `lista_de_itens` leaks.
- If client input fails, `notinha` and `lista_de_itens` leak.
- On early EOF, `continue` abandons both allocations.
- If list insertion fails, the note, client, and item-list container leak.

Do not scatter ad hoc returns through a multi-allocation constructor. Build the complete note in a dedicated constructor, initialize every pointer to `NULL`, and use one cleanup path. Transfer ownership to the list only after insertion succeeds.

A sound ownership transition should be explicit:

```text
constructor owns object
        |
        | successful list insertion
        v
list owns object
```

If insertion fails, the constructor or caller still owns the object and must destroy it.

### 4. `ler_campo` can dereference `NULL` and leave a dangling output

`src/stringo.c:110-128` checks `prompt`, but not `str`. If `str == NULL`, the function skips the destruction block and later executes `*str = input.string`.

When `str` is valid, the function destroys `*str` but does not assign `NULL` to it. If input then fails, the caller retains a dangling pointer.

The required contract is:

- Reject `prompt == NULL` or `str == NULL` before dereferencing anything.
- After destroying the old value, immediately set `*str = NULL`.
- On every failure, leave `*str == NULL`.
- Only assign the new string after the read succeeds.

Also decide whether this function replaces an existing string or only initializes an empty destination. Hiding destructive replacement inside a function named “read field” makes ownership harder to reason about.

### 5. `list_ins_prev` dereferences a failed allocation

`src/list.c:129-131` calls `malloc` and immediately writes through the result. If allocation fails, the error path is a segmentation fault. This is exactly the kind of defect your `LE_FAILED_MALLOC` enum claims to handle.

Checking some allocations while omitting another in the same module means the abstraction cannot be trusted.

### 6. `list_remove` does not validate all required arguments

`src/list.c:48` writes through `data`, but the function never checks whether `data == NULL`. It also accepts any non-null `Node *`; it does not establish that the node belongs to the supplied list. Passing a foreign node can corrupt two structures.

Choose and document a contract:

- Either `data` is mandatory and must be validated;
- or `data` is optional and the function must not dereference it when null.

For node membership, either validate membership or explicitly make it a caller precondition and use assertions in debug builds. Do not leave the contract accidental.

### 7. `list_destroy` can loop forever on an internal failure

`src/list.c:28-31` loops while size is nonzero, but ignores a failed `list_remove`. If removal fails without decreasing size, destruction never terminates.

A destructor must have a deliberate policy for impossible internal states. At minimum, capture the status and stop instead of spinning. Better still, make list invariants strong enough that normal destruction cannot fail, and reserve assertions or a corruption status for violated invariants.

### 8. `stringo_trim_capacity` mishandles successful `realloc`

At `src/stringo.c:76`, `realloc` may move the allocation. The returned pointer is stored in `temp` and then discarded. The object's old `data` pointer may therefore become dangling while the new allocation leaks. `capacity` is not updated either.

This function is currently unused and absent from the public header, but dead unsafe code is still unsafe code. Correct it and test it, or remove it. Do not keep land mines for future you.

## Your error propagation is mostly theater

### Magic-number tagging makes `AddItemClienteResult` fragile

`AddItemClienteResult.error_type` uses undocumented numbers `0`, `1`, `2`, `3`, and `4`. The producer and consumer already disagree:

- `adicionar_item_cliente` uses type `1` for an item-input failure.
- `main` interprets type `1` as a null-argument logic failure.
- Types `2` and `3` are not handled by `main` at all.
- With no `default`, an unhandled failure falls through to “O item foi adicionado com sucesso”.
- `main` refers to `DC_GENERIC_FAIL`, which does not exist.

This is not robust error handling. It is an untyped protocol encoded in folklore.

Use a named enum as the tag. If you truly need details from different subsystems, use a tagged union whose active member is defined by that enum. Never put unrelated status enums into a struct, fill inactive fields with literal zero, and hope every caller remembers what a `short` means.

For this small program, a simpler design is probably better: define one operation-level status that describes what the caller can act on, such as success, invalid argument, input ended, invalid input, allocation failure, client not found, and internal corruption. Preserve a nested cause only when the caller or diagnostic log genuinely needs it.

### You destroy useful error information

`pega_dados_item` maps every string failure to `DI_STRINGO_ERROR` and every integer failure to `DI_INT_ERROR`. That erases whether the cause was EOF, invalid text, overflow, a null argument, or allocation failure.

Map errors by recovery action, not merely by the module where they happened:

- Invalid user input: tell the user what was wrong and retry that field.
- EOF: treat it as cancellation or clean termination, according to a documented policy.
- Allocation failure: clean up and terminate with failure.
- Invalid argument or broken invariant: report an internal programming error and terminate with failure.
- Not found: report it and return to the menu.

If two failures require different caller behavior, they must not collapse into the same status.

### The caller does not exhaustively handle results

Every switch over an error enum or operation tag must be exhaustive. Compile with enum-switch warnings and make unexpected values an explicit internal-error path. Do not silently continue and do not print success after an unknown status.

In `src/main.c:134-138`, `SE_ARG_IS_NULL` falls through into `SE_GENERIC_FAIL`, and neither case exits or retries. Execution continues into another operation despite having announced a fatal logic error. A message is not control flow.

## Input handling is confused

You mix `getchar`, `fgets`, and manual `buffer_flush` calls without a single model for who consumes the newline.

`stringo_get_input` already consumes the newline. `take_int` normally consumes it through `fgets`. Calling `buffer_flush` before every integer read in `pega_dados_item` can therefore block waiting for more input or discard the next intended line.

`take_int` also has these defects:

- `fgets == NULL` collapses EOF and I/O error into `PARSE_GENERIC_FAIL`.
- A line longer than 99 characters remains partially in `stdin`, poisoning the next read.
- Its switch simply reconstructs the exact result returned by `parse_int`; this adds noise without behavior.
- Parsing and stream-reading failures are mixed into one enum even though they are different layers.

Pick one line-reading abstraction for the entire program. Read a complete line dynamically, then parse it. Distinguish:

- clean line read;
- EOF before data;
- stream I/O error (`ferror`);
- allocation failure;
- empty line;
- invalid syntax;
- numeric range overflow.

Do not “flush” input blindly. Consume a known remainder only when you have detected truncation.

## Error reporting and process status are dishonest

All messages use `printf`, including fatal diagnostics. Errors belong on `stderr`, normally via `fprintf(stderr, ...)`. Standard output should remain usable as program output.

More seriously, every path through `clearall` returns `0`, including fatal errors. That tells the operating system, a test runner, or a script that the program succeeded. The initial allocation failures return `-1`, which is also poor style for a portable process status.

Use `EXIT_SUCCESS` for successful user-requested termination and `EXIT_FAILURE` for fatal failure. Track the intended exit status separately from cleanup. Cleanup should not decide whether execution succeeded.

Low-level modules such as `list.c` and `stringo.c` should generally return status and avoid printing. The application boundary—usually `main` or a UI function—decides how to phrase the message. `adicionar_item_cliente` currently both prints and returns an error, encouraging duplicate or contradictory reporting.

A useful diagnostic contains context and cause. “Falha lógica desconhecida” is nearly worthless. Prefer information equivalent to:

```text
cannot add item: allocating list node failed
cannot read quantity: value is outside the int range
internal error: list_remove received a null output pointer
```

Do not expose implementation jargon such as “Early EOF” to an ordinary user. Keep technical diagnostics for developers and give the user a clear action.

## Public functions need real contracts

Several public functions dereference arguments without validation, including `imprimir_notas_fiscais` and `procura_cliente`. `procura_cliente` also fails to advance `node`, so it repeatedly checks only the first note. That is primarily a logic bug, but it demonstrates why “not found” cannot be trusted as an error result.

For each declaration in `include/`, document:

- nullable and non-nullable parameters;
- ownership transfer;
- valid object state required on entry;
- output state on success and failure;
- every returned status;
- whether the function writes diagnostics;
- whether the function is safe on a partially initialized object.

Do not defensively check everything at every layer without thought. Public boundaries should validate caller-controlled inputs. Private helpers may rely on documented invariants and assert them. What matters is that the choice is explicit.

## Recommended error model

Do not create one enum per function just because C lets you. Start with a small application-wide status vocabulary based on recovery behavior. For example:

```c
typedef enum {
    NF_OK = 0,
    NF_ERR_INVALID_ARGUMENT,
    NF_ERR_OUT_OF_MEMORY,
    NF_ERR_END_OF_INPUT,
    NF_ERR_IO,
    NF_ERR_INVALID_INPUT,
    NF_ERR_OUT_OF_RANGE,
    NF_ERR_NOT_FOUND,
    NF_ERR_CORRUPT_STATE
} NfStatus;
```

This is an example, not a command to paste blindly. If a module needs richer internal detail, preserve it in a module-specific result, then deliberately map it at the boundary. The caller should never need magic numbers to discover which field is valid.

Use consistent result shapes:

```c
typedef struct {
    NfStatus status;
    DadosCliente *value;
} DadosClienteResult;
```

The invariants should be boring and universal:

- On `NF_OK`, `value` is non-null and ownership is transferred to the caller.
- On failure, `value` is null.
- A function does not print an error merely because it returns one.
- A caller handles the status before touching the value.

For functions whose only useful output is success or failure, return `NfStatus` directly. For functions that produce a value, use a result struct or an output parameter—consistently, not randomly.

## Cleanup pattern you should use

C does not provide automatic destructors. Stop pretending early returns are free. For a function that acquires several resources, use a single failure path and make every destructor null-safe.

Conceptually:

```c
NfStatus nota_create(NotaFiscal **out) {
    NfStatus status = NF_ERR_CORRUPT_STATE;
    NotaFiscal *nota = NULL;

    if (out == NULL) {
        return NF_ERR_INVALID_ARGUMENT;
    }
    *out = NULL;

    /* Allocate and initialize one resource at a time. */
    /* On failure: set status, then go to cleanup. */

    *out = nota;
    return NF_OK;

cleanup:
    nota_fiscal_destroy(nota);
    return status;
}
```

One `goto cleanup` in C is often cleaner and safer than six duplicated partial-cleanup branches. The bad use of `goto` is tangled control flow; centralized cleanup is a standard, disciplined use.

After transferring ownership, set the local pointer to `NULL` if later cleanup might otherwise destroy it twice.

## Fix order

Follow this order. If you skip ahead, later testing will rest on broken foundations.

### Phase 1: restore a trustworthy build

1. Remove the C++-only include or replace it with the correct C header if assertions are actually used.
2. Replace the nonexistent error identifier and eliminate magic-number error tags.
3. Require a clean build with all existing warnings treated as errors.

Exit criterion: a clean rebuild succeeds from an empty `obj/` and `build/`.

### Phase 2: define ownership and destruction

1. Draw the ownership graph for `Caixa`, `NotaFiscal`, `DadosCliente`, item lists, and strings.
2. Keep exactly one destructor per owned type.
3. Make every destructor accept `NULL` and fully destroy one object.
4. Make constructors return `NULL` values on failure.
5. Audit every ownership transfer at list insertion.

Exit criterion: creating and destroying an empty note, a partially built note, and a complete note produces no sanitizer findings or leaks.

### Phase 3: repair error types and propagation

1. Define statuses by caller action.
2. Replace numeric tags with named enum constants.
3. Preserve causes that change recovery behavior.
4. Make every result switch exhaustive.
5. Ensure no failure path reaches a success message.

Exit criterion: every public failure status has a test that proves the caller takes the intended path.

### Phase 4: make input one coherent subsystem

1. Use one full-line reader.
2. Separate stream errors from parsing errors.
3. Remove blind flushing.
4. Retry only recoverable input mistakes.
5. Treat EOF consistently across every menu and field.

Exit criterion: empty input, malformed numbers, huge numbers, lines longer than the buffer, EOF, and simulated I/O failure all behave predictably.

### Phase 5: make diagnostics and exits truthful

1. Print errors to `stderr`.
2. Let low-level functions return errors instead of choosing UI text.
3. Return `EXIT_FAILURE` after fatal failures.
4. Return `EXIT_SUCCESS` only for actual success.
5. Include operation context in developer-facing diagnostics.

Exit criterion: shell scripts and tests can distinguish success from failure using the process exit code alone.

## Tests you need before claiming this is fixed

Happy-path manual testing is not evidence of correct error handling. Failure paths must be forced.

At minimum, add tests for:

- Every allocator failing at every allocation site, one at a time.
- Every constructor failing after each successfully acquired child resource.
- Insertion failure after an object has been fully constructed.
- Destroying null, empty, partial, and complete objects.
- `list_remove` with null arguments, an empty list, head, tail, middle, and a foreign node.
- EOF before any input and EOF after partial input.
- `ferror` separately from EOF.
- Empty, invalid, boundary, underflowing, and overflowing integers.
- An overlong input line followed by a valid line.
- Client lookup for first, middle, last, and absent clients.
- Every operation-level error producing a failure result and never a success message.
- Fatal paths returning a nonzero process status.

Run with AddressSanitizer and UndefinedBehaviorSanitizer, and add LeakSanitizer or Valgrind where available. Sanitizers are not a substitute for contracts, but they are excellent at proving that your cleanup assumptions were wrong.

## Review checklist for every future function

Before considering a C function finished, answer all of these:

- Are all required pointers validated before dereference?
- Is every allocation checked before use?
- Can any size calculation or capacity growth overflow?
- Are all fields initialized before cleanup can inspect them?
- Is every acquired resource released on every later failure?
- Is ownership transfer documented and performed exactly once?
- Are failed pointer results always null?
- Does the caller inspect status before reading the value?
- Are error categories based on recovery behavior?
- Is detailed cause preserved where it matters?
- Can an unhandled status accidentally become success?
- Are diagnostics emitted at one appropriate layer?
- Does fatal failure reach `EXIT_FAILURE`?
- Can the failure path itself crash, leak, double-free, or loop forever?
- Is there a test that forces this failure rather than hoping it occurs?

## Final judgment

Right now, your error-handling code is more ambitious than disciplined. You have built a vocabulary of failure without building reliable failure semantics. The result is code that can announce fatal errors and continue, return freed pointers, leak partial objects, crash while handling allocation failure, and report fatal termination as success.

That would be unacceptable in a serious company because failure paths are not secondary paths. In C, they are where the program proves whether its author actually understands object lifetime and control flow.

Your next step is not to invent another enum. Make ownership explicit, make cleanup total, make status handling exhaustive, and test the failures on purpose. When those behaviors become boring and predictable, your error handling will finally be credible.
