#include "context.h"
#include "internals.h"

void NASM64_Context_init(NASM64_Context **c, NASM64_Context *parent) {
  *c = NEW(NASM64_Context);
  (*c)->parent = parent;
  list_Symbol_init(&(*c)->symbols);
}

bool NASM64_Context_rsearch(NASM64_Context *ctx, const char *symbol_name) {

  int max = ctx->symbols->count;
  for (int c = 0; c < max; c++) {
    if (strcmp(list_Symbol_get(ctx->symbols, c)->name, symbol_name) == 0) {
      return true;
    }
  }
  return false;
}

void NASM64_Context_add(NASM64_Context *ctx, Symbol *sym) {
  list_Symbol_add(ctx->symbols, sym);
}
