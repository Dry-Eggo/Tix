#include "types.h"

#include <stdbool.h>
#include <string.h>

const char *Type_toraw(Type *t) {
  switch (t->base) {
  case TTYPE_I8:
    return "i8";
  case TTYPE_I16:
    return "i16";
  case TTYPE_I32:
    return "i32";
  case TTYPE_I64:
    return "i64";
  case TTYPE_VOID:
    return "void";
  }
}

bool Type_match(Type t1, Type t2) {
  // basic match for now: implement cast-allowed later
  if (t1.base != t2.base)
    return false;
  return true;
}

Type Type_create_i32() {
  Type t;
  t.base = TTYPE_I32;
  t.is_mut = false;
  t.is_ptr = false;
  t.size_in_bytes = 4;
  t.is_signed  = false;
  return t;
}
Type Type_create_void() {
  Type t;
  t.is_mut = true;
  t.base = TTYPE_VOID;
  t.is_ptr = false;
  t.size_in_bytes = 0;
  return t;
}
