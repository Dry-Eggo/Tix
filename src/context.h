#pragma once

#include "symbol.h"
#include <stddef.h>
#include <stdlib.h>
TIX_DYN_LIST(Symbol, Symbol);
typedef struct _Ctx {
  struct _Ctx *parent;
  list_Symbol *symbols;
} NASM64_Context;

void NASM64_Context_init(NASM64_Context **, NASM64_Context *);
/*
  Perform a recursive search for @symbol_name
  searches its symbol list for the symbol and also engages a recursive search on
  its parent
  @return : bool - true if found else false
*/
bool NASM64_Context_rsearch(NASM64_Context *, const char *symbol_name);
/*
  Perform a Non-Recursive search for @symbol_name
  searches its symbol list for the symbol
  @return : bool - true if found else false
*/
bool NASM64_Context_search(NASM64_Context *, const char *symbol_name);
void NASM64_Context_add(NASM64_Context*, Symbol* sym);
