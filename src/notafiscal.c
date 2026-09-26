#include <stdlib.h>

#include "../include/list.h"
#include "../include/notafiscal.h"
#include "../include/operacoes_clientes.h"

void nota_fiscal_destroy(void *data) {
    if (data == NULL) {
        return;
    }

    NotaFiscal *nf = (NotaFiscal*) data;

    dados_cliente_destroy(nf->cliente);
    // list_destroy(nf->item);

    free(nf);
    nf = NULL;
    return;
}

void nf_item_destroy(DadosItem *item) {
    if (item == NULL) {
        return;
    }

    stringo_destroy(item->descricao);

    free(item);
    return;
}
