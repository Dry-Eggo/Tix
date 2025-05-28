#include "types.h"
#include <stdlib.h>
#include <string.h>

const char *Type_toraw(Type *t) {
  switch (t->base) {
  case TTYPE_I8:
    return strdup("i8");
  case TTYPE_I16:
    return strdup("i16");
  case TTYPE_I32:
    return strdup("i32");
  case TTYPE_I64:
    return strdup("i64");
  }
}

Type *Type_create_i32() {
  Type *t = malloc(sizeof(Type));
  t->base = TTYPE_I32;
  t->is_mut = false;
  t->is_ptr = false;
  return t;
}
