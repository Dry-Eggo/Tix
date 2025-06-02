#include "types.h"

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
    case TTYPE_VOID:
      return strdup("void");
  }
}

Type Type_create_i32() {
  Type t;
  t.base = TTYPE_I32;
  t.is_mut = false;
  t.is_ptr = false;
  t.size_in_bytes = 4;
  return t;
}
