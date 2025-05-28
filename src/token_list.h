#ifndef TIX_TOKEN_LIST
#define TIX_TOKEN_LIST

#include "lexer.h"
typedef struct {
  Token *tokens;
  int count;
  int cap;
} TokenList;

// return 0 on success, -1 on failure
int token_list_init(TokenList *tl);

// appends a token to end of list
int token_list_add(TokenList *tl, Token token);

// retrives a token from a list
const Token *token_list_get(TokenList *tl, int index);

// frees all tokens
void token_list_free(TokenList *tl);

// grows the list
void token_list_grow(TokenList *tl);

#endif
