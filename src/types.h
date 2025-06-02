
#ifndef TIX_TYPES
#define TIX_TYPES
#include <stdbool.h>
typedef enum {
  TTYPE_I32,
  TTYPE_I8,
  TTYPE_I16,
  TTYPE_I64,
  TTYPE_U32,
  TTYPE_U8,
  TTYPE_U16,
  TTYPE_U64,
  TTYPE_VOID
} BaseType;
typedef struct Type {
  BaseType base;
  bool is_ptr;
  bool is_mut;
  int size_in_bytes;
} Type;

const char *Type_toraw(Type *);
bool Type_match(Type t1, Type t2);
Type Type_create_i32();
Type Type_create_void();
#endif
