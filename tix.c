#include "src/ir/ir_gen.h"
#include "src/lexer.h"
#include "src/node.h"
#include "src/parser.h"
#include "src/token_list.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: ticx <source-file>\n");
    return EXIT_FAILURE;
  }
  TLexer lexer;
  if (tix_lexer_init(&lexer, argv[1]) == 0) {
    return EXIT_FAILURE;
  }
  Token token;
  TokenList tokens;
  token_list_init(&tokens);
  do {
    token = tix_lexer_next_token(&lexer);
    // tix_token_print(&token);
    token_list_add(&tokens, token);
  } while (token.kind != TEOF);
  TParser p;
  Program pr;
  program_init(&pr);
  parser_init(&p, &tokens);
  parser_parse(&p, &pr);
  Node_Walker walker;
  Node_init(&walker, &pr);
  Env_init();
  Node_generate(&walker);
  Env_close();
  return EXIT_SUCCESS;
}
