
#ifndef TIX_NODES
#define TIX_NODES

#include "lexer.h"
#include "lists/lists.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
typedef enum {
  TEXPR_BINEXPR,
  TEXPR_UNOPEXPR,
  TEXPR_PREINCEXPR,
  TEXPR_POSTINCEXPR,
  TEXPR_EXPRINT,
  TEXPR_EXPRIDENT,
  TEXPR_EXPRSTR,
  TEXPR_FUNCCALL,
  TEXPR_RETURN,
} ExprKind;

struct BinOp {
  enum TokenKind op;
  struct Expr *lhs;
  struct Expr *rhs;
};
struct UnaryOp {
  enum TokenKind op;
  struct Expr *expr;
};
struct FCall {
  struct Expr *callee;
  struct list_Expr *args;
  size_t arg_count;
};
typedef struct Expr {
  ExprKind kind;
  Span span;
  union {
    int64_t int_value;
    const char *string_value;
    const char *ident_name;
    struct BinOp binary_op;
    struct UnaryOp unop;
    struct FCall call;
  };
} Expr;
TIX_DYN_LIST(Expr, Expr)

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

struct LetStmt {
  const char *name;
  Type type;
  struct Expr *init;
};
typedef struct Stmt {
  Stmtkind kind;
  Span span;
  union {
    struct LetStmt letstmt;
    struct Expr *expr;
    struct list_Stmt *statements;
  } stmt;
} Stmt;

TIX_DYN_LIST(Stmt, Stmt);
typedef enum {
  ITEM_FN,
  ITEM_STRUCT,
  ITEM_TYPEDEF,
  ITEM_IMPORT,
  ITEM_GENLIST,
  ITEM_EXTRN,
} Itemkind;

typedef struct Field {
  const char *name;
  Type *type;
  struct Expr *init;
} Field;

typedef struct Param {
  const char *name;
  Type type;
  struct Expr *init;
} Param;

TIX_DYN_LIST(Param, Param)
struct FnStmt {
  const char *name;
  list_Param *param;
  struct Type return_type;
  struct Stmt *body;
  const Token *Doc;
};
struct StructStmt {
  const char *name;
  struct Field **field;
  size_t field_count;
};
struct Import {
  const char *path;
};

typedef enum {
  EXTERN_FN,
} ExternKind;
struct ExternStmt {
  ExternKind kind;
  union {
    struct FnStmt fn;
  } symbol;
};
typedef struct Item {
  Itemkind kind;
  Span span;
  union {
    struct FnStmt fn;
    struct ExternStmt extrn;
    struct StructStmt stuct_stmt;
    struct Import import;
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
Expr *create_ident(const char *name, Span span);
Expr *create_func_call(Expr *callee, list_Expr *args, Span span);
Node *create_nodei(Item *i);
Node *create_nodes(Stmt *s);
Node *create_nodee(Expr *e);
Expr *create_binop(enum TokenKind t, Expr *lhs, Expr *rhs);
Expr *create_unop(enum TokenKind op, Expr *expr, Span span);
Expr *create_postinc(enum TokenKind op, Expr *expr, Span span);
Expr *create_preinc(enum TokenKind op, Expr *expr, Span span);
#endif
