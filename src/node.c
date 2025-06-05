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

Expr *create_unop(enum TokenKind op, Expr *expr, Span span) {
  Expr *unop = NEW(Expr);
  unop->kind = TEXPR_UNOPEXPR;
  unop->unop.op = op;
  unop->unop.expr = expr;
  unop->span = span;
  return unop;
}

Expr *create_preinc(enum TokenKind op, Expr *expr, Span span) {
  Expr *preinc = NEW(Expr);
  preinc->kind = TEXPR_PREINCEXPR;
  preinc->unop.op = op;
  preinc->unop.expr = expr;
  preinc->span = span;
  return preinc;
}
Expr *create_postinc(enum TokenKind op, Expr *expr, Span span) {
  Expr *postinc = NEW(Expr);
  postinc->kind = TEXPR_POSTINCEXPR;
  postinc->unop.op = op;
  postinc->unop.expr = expr;
  postinc->span = span;
  return postinc;
}
Expr *create_intlit(int64_t i, Span span) {
  Expr *ie = (Expr *)malloc(sizeof(Expr));
  ie->kind = TEXPR_EXPRINT;
  ie->span = span;
  ie->int_value = i;
  return ie;
}

Expr *create_strlit(const char *val, Span span) {
  Expr *se = NEW(Expr);
  se->kind = TEXPR_EXPRSTR;
  se->span = span;
  se->string_value = val;
  return se;
}

Expr *create_ident(const char *name, Span span) {
  Expr *ident = NEW(Expr);
  ident->ident_name = strdup(name);
  ident->kind = TEXPR_EXPRIDENT;
  ident->span = span;
  return ident;
}

Expr *create_func_call(Expr *callee, list_Expr *args, Span span) {
  Expr *func_call = NEW(Expr);
  func_call->kind = TEXPR_FUNCCALL;
  func_call->call.callee = callee;
  func_call->call.args = args;
  func_call->call.arg_count = args->count;
  return func_call;
}
