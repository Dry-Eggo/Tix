#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void lexer_advance(TLexer *lexer) {
  if (lexer->pos >= lexer->stream_size) {
    int br = fread(lexer->stream, 1, lexer->stream_cap, lexer->source_file);
    if (br == 0) {
      lexer->current_char = EOF;
      return;
    }
    lexer->stream_size = br;
    lexer->pos = 0;
  }
  lexer->current_char = lexer->stream[lexer->pos];
  lexer->pos++;
  if (lexer->current_char == '\n') {
    lexer->line++;
    lexer->col = 1;
  } else {
    lexer->col++;
  }
}

static char lexer_peek(TLexer *lexer) {
  if (lexer->pos >= lexer->stream_size && feof(lexer->source_file)) {
    return EOF;
  }
  if (lexer->pos >= lexer->stream_size)
    return EOF;
  return lexer->stream[lexer->pos];
}

/* skip whitespace */
static void lexer_skipws(TLexer *lexer) {
  while (isspace(lexer->current_char) && lexer->current_char != EOF) {
    lexer_advance(lexer);
  }
}

/* comments */
static void skip_singleline_comments(TLexer *lexer) {
  while (lexer->current_char != EOF && lexer->current_char != '\n') {
    lexer_advance(lexer);
  }
  if (lexer->current_char == '\n')
    lexer_advance(lexer);
}

static Token create_token(enum TokenKind t, int line, int cols, int cole,
                          char *data) {
  Token tok;
  tok.kind = t;
  tok.data = (data != NULL) ? strdup(data) : NULL;
  Span s;
  s.start = cols;
  s.end = cole;
  tok.span = s;
  tok.line = line;
  return tok;
}

static enum TokenKind token_iskeyword(const char *ident) {
  if (strcmp(ident, "fn") == 0) {
    return TFN;
  } else if (strcmp(ident, "int") == 0) {
    return TINT;
  } else if (strcmp(ident, "if") == 0) {
    return TIF;
  } else if (strcmp(ident, "str") == 0) {
    return TSTR;
  } else if (strcmp(ident, "return") == 0) {
    return TRETURN;
  }
  return TIDENT;
}

static Token lexer_parse_kw_or_ident(TLexer *lexer) {
  char buf[256];
  int i = 0;
  int start = lexer->pos;
  while (isalnum(lexer->current_char) && lexer->current_char != EOF) {
    buf[i++] = lexer->current_char;
    lexer_advance(lexer);
  }
  buf[i] = '\0';
  enum TokenKind kind = token_iskeyword(buf);
  return create_token(kind, lexer->line, start, lexer->pos, buf);
}

int tix_lexer_init(TLexer *lexer, const char *filename) {
  lexer->source_file = fopen(filename, "r");
  if (!lexer->source_file) {
    perror("Error Opening Source File");
    return 0;
  }
  lexer->stream_cap = TIX_MAX_STREAM_SIZE;
  lexer->stream = (char *)malloc(lexer->stream_cap);
  if (!lexer->stream) {
    perror("Error Alocating Lexer Buffer");
    fclose(lexer->source_file);
    return 0;
  }
  lexer->col = 0;
  lexer->line = 1;
  lexer->pos = 0;
  lexer->current_char = '\n';
  lexer->stream_size = 0;
  lexer_advance(lexer);
  return 1;
}

void tix_lexer_free(TLexer *lexer) {
  if (lexer->source_file) {
    fclose(lexer->source_file);
    lexer->source_file = NULL;
  }
  if (lexer->stream) {
    free(lexer->stream);
    lexer->stream = NULL;
  }
}

void tix_token_free(Token *tok) {
  if (tok && tok->data) {
    free(tok->data);
    tok->data = NULL;
  }
}

Token tix_lexer_next_token(TLexer *lexer) {
  int start_line, start_col;

  while (lexer->current_char != EOF) {
    start_line = lexer->line;
    start_col = lexer->col;

    if (isspace(lexer->current_char)) {
      lexer_skipws(lexer);
      continue;
    }

    if (lexer->current_char == '/') {
      if (lexer_peek(lexer) == '/') {
        lexer_advance(lexer);
        lexer_advance(lexer);
        skip_singleline_comments(lexer);
        continue;
      }
    }
    if (isalpha(lexer->current_char) || lexer->current_char == '_') {
      return lexer_parse_kw_or_ident(lexer);
    }
    if (lexer->current_char == '{') {
      lexer_advance(lexer);
      return create_token(TOBRACE, lexer->line, start_col, lexer->pos, NULL);
    }
    if (lexer->current_char == '}') {
      lexer_advance(lexer);
      return create_token(TCBRACE, lexer->line, start_col, lexer->pos, NULL);
    }
    switch (lexer->current_char) {
    case '=':
      lexer_advance(lexer);
      return create_token(TEQ, lexer->line, start_col, lexer->pos, NULL);
      break;
    case '(':
      lexer_advance(lexer);
      return create_token(TOPAREN, lexer->line, start_col, lexer->pos, NULL);
      break;
    case ')':
      lexer_advance(lexer);
      return create_token(TCPAREN, lexer->line, start_col, lexer->pos, NULL);
    default:
      break;
    }
  }
  return create_token(TEOF, lexer->line, start_col, lexer->pos, NULL);
}
const char *token_tostr(enum TokenKind k) {
  char *name;
  switch (k) {
  case TOPAREN:
    name = "(";
    break;
  case TCPAREN:
    name = ")";
    break;
  case TOBRACE:
    name = "{";
    break;
  default:
    name = "<unimplemented>";
    break;
  }
  return strdup(name);
}
void tix_token_print(const Token *token) {
  printf("Token { %d, ", token->kind);
  printf("%s}\n", (token->data == NULL) ? "<null>" : token->data);
}
