#ifndef TIX_PARSER
#define TIX_PARSER

#include "lexer.h"
#include "node.h"
#include "token_list.h"
typedef struct {
  TokenList *tokens;
  int pos;
  int max;
  char **source;
} TParser;

const Token *parser_peek(TParser *p);
void parser_advance(TParser *p);
int parser_init(TParser *parser, TokenList *tokens);
void parser_parse(TParser *, Program *);
Item *parser_function(TParser *);
Item *parser_parse_extern(TParser*);
void parser_parse_parameter(TParser *, Param **);
Stmt *parser_parse_let(TParser *);
Stmt *parser_block(TParser *);
const char *parser_expect_ident(TParser *p);
/* matches the next token and advances if matched. else error */
void parser_expect(TParser *, enum TokenKind);
Expr *parser_parse_expr(TParser *);
Expr *parser_parse_term(TParser *);
Expr *parser_parse_atom(TParser *);
Type parser_parse_type(TParser *);
static bool parser_is_loop;
static bool parser_is_function;
static bool parser_should_return;
#endif
