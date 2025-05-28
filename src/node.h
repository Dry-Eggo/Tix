
#ifndef TIX_NODES
#define TIX_NODES

#include "lexer.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>
typedef enum {
  TEXPR_BINEXPR,
  TEXPR_EXPRINT,
  TEXPR_EXPRSTR,
  TEXPR_FUNCCALL,
  TEXPR_RETURN,
} ExprKind;

typedef struct Expr {
  ExprKind kind;
  Span span;
  union {
    int64_t int_value;
    const char *string_value;
    const char *ident_name;

    struct {
      enum TokenKind op;
      struct Expr *lhs;
      struct Expr *rhs;
    } binop;
    struct {
      enum TokenKind op;
      struct Expr *expr;
    } unop;
    struct {
      struct Expr *callee;
      struct Expr **args;
      size_t arg_count;
    } call;
  };
} Expr;

typedef enum {
  TSTMT_LET,
  TSTMT_MUT,
  TSTMT_EXPR,
  TSTMT_IF,
  TSTMT_BLOCK,
  TSTMT_LOOP,
  TSTMT_WHILE,
  TSTMT_BREAK,
  TSTMT_CONTINUE,
} Stmtkind;

typedef struct Stmt {
  Stmtkind kind;
  Span span;
  union {
    const char *name;
    struct Expr *init;
  } let_stmt;
  union {
    struct Expr *expr;
  } expr_stmt;
} Stmt;

typedef enum {
  ITEM_FN,
  ITEM_STRUCT,
  ITEM_TYPEDEF,
  ITEM_IMPORT,
  ITEM_GENLIST,
} Itemkind;

typedef struct Field {
  const char *name;
  Type *type;
  struct Expr *init;
} Field;

typedef struct Param {
  const char *name;
  Type *type;
  struct Expr *init;
} Param;

typedef struct Item {
  Itemkind kind;
  Span span;
  union {
    struct {
      const char *name;
      struct Param **param;
      size_t param_count;
      struct Type *return_type;
      struct Stmt *body;
    } fn;
    struct {
      const char *name;
      struct Field **field;
      size_t field_count;
    } struct_decl;
    struct {
      const char *path;
    } import;
  };
} Item;
typedef enum { TNODE_EXPR, TNODE_STMT, TNODE_ITEM, TNODE_PROG } NodeKind;

typedef struct Node {
  NodeKind type;
  union {
    struct Expr *expr;
    struct Stmt *stmt;
    struct Item *item;
  } node;
} Node;

typedef struct Program {
  Node **nodes;
  int count;
  int cap;
} Program;

int program_init(Program *);
int program_grow(Program *);
int program_add(Program *, Node *);

Expr *create_intlit(int64_t val, Span span);
Expr *create_strlit(const char *val, Span span);
Node *create_nodei(Item *i);
Node *create_nodes(Stmt *s);
Node *create_nodee(Expr *e);

#endif
