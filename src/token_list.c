#include "token_list.h"
#include "lexer.h"
#include <stdlib.h>

int token_list_init(TokenList *tl) {
  tl->tokens = malloc(64 * sizeof(Token));
  tl->cap = 64;
  tl->count = 0;
  return 0;
}

int token_list_add(TokenList *tl, Token token) {
  if (tl->count >= tl->cap)
    token_list_grow(tl);
  tl->tokens[tl->count++] = token;
  return 0;
}

const Token *token_list_get(TokenList *tl, int in) {
  if (in >= tl->cap)
    return NULL;
  return &tl->tokens[in];
}

void token_list_grow(TokenList *tl) {
  tl->tokens = realloc(tl->tokens, sizeof(Token) * (tl->cap * 2));
  tl->cap *= 2;
}
