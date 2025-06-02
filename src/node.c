#include "node.h"
#include "internals.h"
#include "lexer.h"
#include <stdint.h>
#include <stdlib.h>

int program_init(Program *p) {
  p->nodes = (Node **)malloc(64 * 8);
  if (!p->nodes)
    return -1;
  p->cap = 64;
  p->count = 0;
  return 0;
}
int program_add(Program *p, Node *n) {
  if (p->count >= p->cap)
    program_grow(p);
  p->nodes[p->count++] = n;
  return 0;
}
int program_grow(Program *p) {
  p->nodes = (Node **)realloc(p->nodes, p->cap * 2);
  p->cap *= 2;
  return 0;
}

Node *create_nodei(Item *i) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->node.item = i;
  node->type = TNODE_ITEM;
  return node;
}
Expr *create_binop(enum TokenKind t, Expr *lhs, Expr *rhs) {
  Expr *expr = malloc(sizeof(Expr));
  expr->kind = TEXPR_BINEXPR;
  expr->binary_op.lhs = lhs;
  expr->binary_op.op = t;
  expr->binary_op.rhs = rhs;
  return expr;
}

Expr *create_intlit(int64_t i, Span span) {
  Expr *ie = (Expr *)malloc(sizeof(Expr));
  ie->kind = TEXPR_EXPRINT;
  ie->span = span;
  ie->int_value = i;
  return ie;
}

Expr *create_ident(const char *name, Span span) {
  Expr *ident = NEW(Expr);
  ident->ident_name = strdup(name);
  ident->kind = TEXPR_EXPRIDENT;
  ident->span = span;
  return ident;
}
