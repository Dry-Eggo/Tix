#include "parser.h"
#include "internals.h"
#include "lexer.h"
#include "node.h"
#include "string_builder.h"
#include "token_list.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PNOW(p) token_list_get(p->tokens, p->pos)
#define PNOWT(p) parser_peek(p)->kind
#define PNOWB(p) token_list_get(p->tokens, p->pos - 1)
#define EQ(t1, t2) (t1 == t2)

int parser_init(TParser *parser, TokenList *tokens) {
  if (!tokens)
    return -1;
  parser->tokens = tokens;
  parser->max = tokens->count;
  parser->pos = 0;
  return 0;
}

const Token *parser_peek(TParser *parser) {
  if (parser->pos >= parser->max)
    return NULL;

  return token_list_get(parser->tokens, parser->pos);
}

void parser_advance(TParser *p) { p->pos++; }

void parser_parse(TParser *p, Program *program) {
  while (p->pos < p->max) {
    if (PNOW(p)->kind == TFN) {
      program_add(program, create_nodei(parser_function(p)));
      continue;
    } else if (PNOWT(p) == TEXTRN) {
      program_add(program, create_nodei(parser_parse_extern(p)));
    }
    parser_advance(p);
  }
}
Stmt *parser_parse_let(TParser *p) {
  int start = PNOW(p)->span.start;
  int line = PNOW(p)->span.line;
  parser_advance(p); // skip 'let'
  const char *ident = parser_expect_ident(p);
  parser_expect(p, TCOL);
  Type type = parser_parse_type(p);
  type.is_mut = false;
  Stmt *letstmt = malloc(sizeof(Stmt));
  letstmt->stmt.letstmt.init = NULL;
  if (PNOWT(p) == TEQ) {
    parser_expect(p, TEQ);
    Expr *value = parser_parse_expr(p);
    letstmt->stmt.letstmt.init = value;
  }
  letstmt->kind = TSTMT_LET;
  letstmt->stmt.letstmt.name = strdup(ident);
  letstmt->stmt.letstmt.type = type;
  parser_expect(p, TSEMI);
  letstmt->span =
      (Span){.start = start, .end = PNOW(p)->span.end, .line = line};
  return letstmt;
}

Type parser_parse_type(TParser *p) {
  switch (PNOWT(p)) {
  case TI32: {
    Type ty;
    ty = Type_create_i32();
    parser_advance(p);
    return ty;
  }
  case TU8: {
    Type ty = Type_create_i32(); // TODO: Dont be lazy man.
    ty.base = TTYPE_U8;          // really n**ga
    parser_advance(p);
    ty.size_in_bytes = 1;
    return ty;
  }
  case TMUL: {
    parser_advance(p);
    Type inner = parser_parse_type(p);
    inner.is_ptr = true;
    inner.size_in_bytes = 8;
    TIX_LOG(stdout, INFO, "Parsed pointer type");
    return inner;
  }
  case TVOID: {
    parser_advance(p);
    return Type_create_void();
  }
  default:
    tix_error(PNOWB(p)->span, "Unexpected type", p->source, NULL);
    exit(1); /* shouldn't reach here */
  }
}
Stmt *parser_parse_block(TParser *p) {
  Stmt *blockStmt = (Stmt *)malloc(sizeof(Stmt));
  blockStmt->kind = TSTMT_BLOCK;
  list_Stmt_init(&blockStmt->stmt.statements);
  while (PNOWT(p) != TCBRACE) {
    switch (PNOWT(p)) {
    case TLET: {
      list_Stmt_add(blockStmt->stmt.statements, parser_parse_let(p));
    }
      continue;
    default: {
      Stmt *exprStmt = NEW(Stmt);
      exprStmt->stmt.expr = parser_parse_expr(p);
      exprStmt->kind = TSTMT_EXPR;
      list_Stmt_add(blockStmt->stmt.statements, exprStmt);
      parser_expect(p, TSEMI);
    } break;
    }
  }
  return blockStmt;
}

Expr *parser_parse_expr(TParser *p) {
  Expr *lhs = parser_parse_term(p);
  while (PNOWT(p) == TADD || PNOWT(p) == TSUB) {
    enum TokenKind op = PNOWT(p);
    parser_advance(p);
    Expr *rhs = parser_parse_term(p);
    lhs = create_binop(op, lhs, rhs);
  }
  return lhs;
}

Expr *parser_parse_term(TParser *p) {
  Expr *lhs = parser_parse_atom(p);
  while (PNOWT(p) == (TMUL | TDIV)) {
    enum TokenKind op = PNOWT(p);
    parser_advance(p);
    Expr *rhs = parser_parse_atom(p);
    lhs = create_binop(op, lhs, rhs);
  }
  return lhs;
}

Expr *parser_parse_atom(TParser *p) {
  switch (PNOWT(p)) {
  case TNUMBER: {
    char *endptr = NULL;
    Expr *num =
        create_intlit(strtoll(PNOW(p)->data, &endptr, 10), PNOW(p)->span);
    parser_advance(p);
    if (*endptr != '\0') {
      tix_error(PNOW(p)->span, "Non-Numeric Characters in integer", p->source,
                NULL);
    }
    return num;
  }
  case TSTR: {
    Expr *str = create_strlit(PNOW(p)->data, PNOW(p)->span);
    parser_advance(p);
    return str;
  } break;
  case TIDENT: {
    int start = PNOW(p)->span.start;
    int line = PNOW(p)->span.line;
    char *name = PNOW(p)->data;
    Expr *ident = create_ident(name, PNOW(p)->span);
    parser_advance(p);
    if (EQ(PNOWT(p), TOPAREN)) {
      parser_advance(p);
      list_Expr *args;
      list_Expr_init(&args);
      while (!EQ(PNOWT(p), TCPAREN)) {
        list_Expr_add(args, parser_parse_expr(p));
        if (EQ(TCOMMA, PNOWT(p))) {
          parser_advance(p);
          continue;
        }
      }
      if (EQ(PNOWT(p), TCPAREN))
        parser_expect(p, TCPAREN);

      int end = PNOW(p)->span.end;
      Expr *fccall = create_func_call(
          ident, args, (Span){.start = start, .end = end, .line = line});
      return fccall;
    }
    return ident;
  } break;
  case TOPAREN: {
    parser_advance(p);
    Expr *inner = parser_parse_expr(p);
    parser_expect(p, TCPAREN);
    return inner;
  } break;
  case TSUB: {
    enum TokenKind op = PNOWT(p);
    parser_advance(p);
    Expr *expr = parser_parse_expr(p);
    Expr *unop = create_unop(op, expr, expr->span);
    return unop;
  } break;
  default:
    tix_error(PNOW(p)->span, "Invalid Expr", p->source, NULL);
  }
}
Item *parser_parse_extern(TParser *p) {
  parser_advance(p);
  // TODO: add support for brace enclosed list of extrns
  switch (PNOWT(p)) {
  case TFN: {
    Item *fn = parser_function(p);
    Item *extrn = NEW(Item);
    extrn->kind = ITEM_EXTRN;
    extrn->extrn.symbol.fn = fn->fn;
    extrn->extrn.kind = EXTERN_FN;
    return extrn;
  } break;
  default:
    TIX_LOG(stderr, ERROR, "Item does not support extern");
    exit(1);
  }
  return NULL;
}
Item *parser_function(TParser *p) {
  Item *func_stmt = (Item *)malloc(sizeof(Item));
  Span span = PNOW(p)->span;
  parser_advance(p); // skip 'fn' keyword
  char *name = malloc(64);
  name = strdup(parser_expect_ident(p));
  func_stmt->fn.name = name;
  parser_expect(p, TOPAREN);
  list_Param_init(&func_stmt->fn.param);
  bool has_encountered_default_parameter = false;
  while (!EQ(PNOWT(p), TCPAREN)) {
    Param *param;
    parser_parse_parameter(p, &param);
    if (EQ(PNOWT(p), TCOMMA)) {
      parser_expect(p, TCOMMA);
      list_Param_add(func_stmt->fn.param, param);
      if (param->init && !has_encountered_default_parameter) {
        has_encountered_default_parameter = true;
      }
      if (!param->init && has_encountered_default_parameter) {
        TIX_LOG(stderr, ERROR, "'%s' missing default value", param->name);
        exit(1);
      }
      continue;
    }
    list_Param_add(func_stmt->fn.param, param);
  }
  parser_expect(p, TCPAREN);
  if (!EQ(PNOWT(p), TOBRACE)) {
    func_stmt->fn.return_type = parser_parse_type(p);
  } else {
    func_stmt->fn.return_type = Type_create_void();
  }
  if (EQ(PNOWT(p), TOBRACE)) {
    parser_expect(p, TOBRACE);
    Stmt *blockstmt = parser_parse_block(p);
    func_stmt->fn.body = blockstmt;
    parser_expect(p, TCBRACE);
  }
  func_stmt->kind = ITEM_FN;
  func_stmt->span = span;
  return func_stmt;
}
void parser_parse_parameter(TParser *p, Param **param) {
  *param = NEW(Param);
  const char *param_name = parser_expect_ident(p);
  if (EQ(PNOWT(p), TCOL)) {
    parser_expect(p, TCOL);
    Type ty = parser_parse_type(p);
    (*param)->type = ty;
    if (EQ(PNOWT(p), TEQ)) {
      parser_expect(p, TEQ);
      Expr *param_initializer = parser_parse_expr(p);
      (*param)->init = param_initializer;
    }
  }
  (*param)->name = param_name;
}
const char *parser_expect_ident(TParser *p) {
  if (PNOWT(p) == TIDENT) {
    const Token *t = PNOW(p);
    parser_advance(p);
    return strdup(t->data);
  }
  tix_error(PNOWB(p)->span, "Expected an identifier", p->source, NULL);
  exit(1);
}
void parser_expect(TParser *p, enum TokenKind k) {
  if (PNOW(p)->kind == k) {
    parser_advance(p);
    return;
  }
  StringBuilder *sb;
  SB_init(&sb);
  sbapp(sb, "Expected '%s'", token_tostr(k));
  tix_error(PNOW(p)->span, sb->data, p->source, NULL);
  exit(1);
  return;
}
