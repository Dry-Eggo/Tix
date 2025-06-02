#pragma once
#include "lists/lists.h"
#include "types.h"
typedef struct Symbol {
  struct Type type;
  const char *name;
  bool is_init;
  bool is_ref;
  struct Symbol *references;
  struct Symbol **borrows;
  int borrow_count;
  int offset;
} Symbol;

void Symbol_init(Symbol **, const char *name, struct Type type, int offset);
