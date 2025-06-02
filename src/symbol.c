#include "symbol.h"
#include "internals.h"
#include <stdbool.h>

void Symbol_init(Symbol **sym, const char *name, struct Type type, int offset) {
  *sym = NEW(Symbol);
  (*sym)->name = name;
  (*sym)->borrow_count = 0;
  (*sym)->borrows = NEW(Symbol *);
  (*sym)->is_init = false;
  (*sym)->references = NULL;
  (*sym)->type = type;
  (*sym)->is_ref = false;
  (*sym)->offset = offset;
}
