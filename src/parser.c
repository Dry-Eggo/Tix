#include "parser.h"
#include "internals.h"
#include "lexer.h"
#include "node.h"
#include "token_list.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

#define PNOW(p) token_list_get(p->tokens, p->pos)
#define PNOWT(p) token_list_get(p->tokens, p->pos)->kind
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
    TIX_LOG(stderr, INFO, "This value is initialized");
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
  default:
    tix_error(PNOW(p)->span, "Unexpected type", p->source, NULL);
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
    default:
      tix_error(PNOW(p)->span, "unexpected token", p->source, NULL);
      break;
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
  case TIDENT: {
    char *name = PNOW(p)->data;
    Expr *ident = create_ident(name, PNOW(p)->span);
    parser_advance(p);
    return ident;
  } break;
  }
  case TOPAREN: {
      parser_advance(p);
      Expr* inner = parser_parse_expr(p);
      parser_expect(p, TCPAREN);
      return inner;
  } break;
  default:
    tix_error(PNOW(p)->span, "Invalid Expr", p->source, NULL);
  }
}
Item *parser_function(TParser *p) {
  Item *func_stmt = (Item *)malloc(sizeof(Item));
  parser_advance(p); // skip 'fn' keyword
  char *name = malloc(255);
  name = strdup(parser_expect_ident(p));
  func_stmt->fn.name = name;
  func_stmt->fn.return_type = Type_create_i32();
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
  parser_expect(p, TOBRACE);
  Stmt *blockstmt = parser_parse_block(p);
  func_stmt->fn.body = blockstmt;
  parser_expect(p, TCBRACE);
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
  fprintf(stderr, "Expected an identifier, got '%s' instead\n",
          token_tostr(PNOWT(p)));
  exit(1);
}
void parser_expect(TParser *p, enum TokenKind k) {
  if (PNOW(p)->kind == k) {
    parser_advance(p);
    return;
  }
  fprintf(stderr, "Expected '%s' got '%s' instead'\n", token_tostr(k),
          token_tostr(PNOWT(p)));
  exit(1);
  return;
}
