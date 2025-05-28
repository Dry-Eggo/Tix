#include "parser.h"
#include "lexer.h"
#include "node.h"
#include "token_list.h"
#include <stdlib.h>
#include <string.h>

#define PNOW(p) token_list_get(p->tokens, p->pos)
#define PNOWT(p) token_list_get(p->tokens, p->pos)->kind

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

Item *parser_function(TParser *p) {
  Item *func_stmt = (Item *)malloc(sizeof(Item));
  parser_advance(p); // skip 'fn' keyword
  char *name = malloc(255);
  name = strdup(parser_expect_ident(p));
  func_stmt->fn.name = name;
  func_stmt->fn.return_type = Type_create_i32();
  parser_expect(p, TOPAREN);
  parser_expect(p, TCPAREN);
  parser_expect(p, TOBRACE);
  parser_expect(p, TCBRACE);
  printf("parsed function body (%s)\n", func_stmt->fn.name);
  return func_stmt;
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
