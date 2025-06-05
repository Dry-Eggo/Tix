#include "lexer.h"
#include "internals.h"
#include "string_builder.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the lexer
int tix_lexer_init(TLexer *lexer, const char *filename) {
  lexer->stream_cap = TIX_MAX_STREAM_SIZE;
  lexer->stream = (char *)malloc(1024);
  FILE *f = fopen(filename, "r");
  fread(lexer->stream, 1024, 1, f);
  lexer->pos = 0;
  lexer->line = 1;
  lexer->col = 1;
  lexer->current_char = lexer->stream[lexer->pos];
  lexer->stream_size = 0;
  return 0;
}

// Advance lexer by one character
static void lexer_advance(TLexer *lexer) {
  if (lexer->current_char == '\n') {
    lexer->line++;
    lexer->col = 1;
  } else {
    lexer->col++;
  }
  lexer->pos++;
  if (lexer->stream[lexer->pos] != '\0') {
    lexer->current_char = lexer->stream[lexer->pos];
  } else {
    lexer->current_char = EOF;
  }
}

// Helper to create tokens
Token create_token(enum TokenKind type, int start_col, int end_col, int line,
                   const char *text) {
  Token token;
  token.kind = type;
  token.span.start = start_col;
  token.span.end = end_col;
  token.span.line = line;
  token.data = text;
  return token;
}

// Consume whitespace characters (space, tab, carriage return, newline)
void lexer_skip_whitespace(TLexer *lexer) {
  while (lexer->current_char != EOF &&
         (lexer->current_char == ' ' || lexer->current_char == '\t' ||
          lexer->current_char == '\r' || lexer->current_char == '\n')) {
    lexer_advance(lexer);
  }
}

// Lex an integer token (simple digits only)
Token lexer_integer(TLexer *lexer) {
  int start_line = lexer->line;
  int start_col = lexer->col;
  start_col -= 1;

  int buffer_len = 32;
  char *buffer = malloc(buffer_len);
  int length = 0;

  while (lexer->current_char != EOF && isdigit(lexer->current_char)) {
    if (length + 1 >= buffer_len) {
      buffer_len *= 2;
      buffer = realloc(buffer, buffer_len);
    }
    buffer[length++] = lexer->current_char;
    lexer_advance(lexer);
  }
  buffer[length] = '\0';

  int end_col = lexer->col - 1;

  Token tok = create_token(TNUMBER, start_col, end_col, start_line, buffer);
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
  } else if (strcmp(ident, "let") == 0) {
    return TLET;
  } else if (strcmp(ident, "i32") == 0) {
    return TI32;
  } else if (strcmp(ident, "i8") == 0) {
    return TI8;
  } else if (strcmp(ident, "i16") == 0) {
    return TI16;
  } else if (strcmp(ident, "i64") == 0) {
    return TI64;
  } else if (strcmp(ident, "u8") == 0) {
    return TU8;
  } else if (strcmp(ident, "u16") == 0) {
    return TU16;
  } else if (strcmp(ident, "u32") == 0) {
    return TU32;
  } else if (strcmp(ident, "u64") == 0) {
    return TU64;
  } else if (strcmp(ident, "void") == 0) {
    return TVOID;
  } else if (strcmp(ident, "extrn") == 0) {
    return TEXTRN;
  }
  return TIDENT;
}
Token lexer_parse_kw_or_ident(TLexer *lexer) {
  int start = lexer->col - 1;
  StringBuilder *ident;
  SB_init(&ident);
  while (lexer->current_char != EOF &&
         (isalnum(lexer->current_char) || lexer->current_char == '_')) {
    sbapp(ident, "%c", lexer->current_char);
    lexer_advance(lexer);
  }
  enum TokenKind kind = token_iskeyword(ident->data);
  return create_token(kind, start, lexer->col - 1, lexer->line, ident->data);
}
// Lex next token
Token tix_lexer_next_token(TLexer *lexer) {
  while (lexer->current_char != EOF) {
    // Skip whitespace first
    if (lexer->current_char == ' ' || lexer->current_char == '\t' ||
        lexer->current_char == '\r' || lexer->current_char == '\n') {
      lexer_skip_whitespace(lexer);
      continue;
    }

    int start_line = lexer->line;
    int start_col = lexer->col - 1;

    if (isdigit(lexer->current_char)) {
      return lexer_integer(lexer);
    }
    if (isalpha(lexer->current_char)) {
      return lexer_parse_kw_or_ident(lexer);
    }
    if (lexer->current_char == '\"') {
      lexer_advance(lexer);
      char buf[255] = {0};
      while (lexer->current_char != '\"') {
        tstrcatf(buf, "%c", lexer->current_char);
        lexer_advance(lexer);
      }
      if (lexer->current_char == '\"')
        lexer_advance(lexer);
      return create_token(TSTR, lexer->line, start_col, lexer->col - 1,
                          strdup(buf));
      *buf = 0;
    }

    char current = lexer->current_char;

    switch (current) {
    case '+':
      lexer_advance(lexer);
      if (lexer->current_char == '+') {
        lexer_advance(lexer);
        return create_token(TADDADD, start_col, lexer->col - 1, start_line,
                            "++");
      }
      return create_token(TADD, start_col, start_col, start_line, "+");
    case '-':
      lexer_advance(lexer);
      if (lexer->current_char == '-') {
        lexer_advance(lexer);
        return create_token(TSUBSUB, start_col, lexer->col - 1, start_line,
                            "--");
      }
      return create_token(TSUB, start_col, start_col, start_line, "-");
    case '*':
      lexer_advance(lexer);
      return create_token(TMUL, start_col, start_col, start_line, "*");
    case '/':
      lexer_advance(lexer);
      if (lexer->current_char == '/') {
        lexer_advance(lexer);
        while (lexer->current_char != '\n') {
          lexer_advance(lexer);
        }
        if (lexer->current_char == '/')
          lexer_advance(lexer);
        continue;
      }
      return create_token(TDIV, start_col, start_col, start_line, "/");
    case '(':
      lexer_advance(lexer);
      return create_token(TOPAREN, start_col, start_col, start_line, "(");
    case ')':
      lexer_advance(lexer);
      return create_token(TCPAREN, start_col, start_col, start_line, ")");
    case ':':
      lexer_advance(lexer);
      return create_token(TCOL, start_col, start_col, start_line, ":");
    case ';':
      lexer_advance(lexer);
      return create_token(TSEMI, start_col, start_col, start_line, ":");
    case '{':
      lexer_advance(lexer);
      return create_token(TOBRACE, start_col, start_col, start_line, ":");
    case '}':
      lexer_advance(lexer);
      return create_token(TCBRACE, start_col, start_col, start_line, ":");
    case '=':
      lexer_advance(lexer);
      return create_token(TEQ, start_col, start_col, start_line, ":");
    default:
    }
  }

  // EOF token at end
  int line = lexer->line;
  int col = lexer->col;
  return create_token(TEOF, col, col, line, "<EOF>");
}

// For testing: print tokens
extern bool TIX_DEBUG_ENABLED;
void print_token(Token tok) {
  if (!TIX_DEBUG_ENABLED)
    return;
  const char *type_str;
  switch (tok.kind) {
  case TNUMBER:
    type_str = "INT";
    break;
  case TADD:
    type_str = "PLUS";
    break;
  case TSUB:
    type_str = "MINUS";
    break;
  case TMUL:
    type_str = "MUL";
    break;
  case TDIV:
    type_str = "DIV";
    break;
  case TOPAREN:
    type_str = "OPAREN";
    break;
  case TCPAREN:
    type_str = "CPAREN";
    break;
  case TEOF:
    type_str = "EOF";
    break;
  case TIDENT:
    type_str = "IDENT";
    break;
  default:
    type_str = "UNKNOWN";
    break;
  }

  printf("Token: %-7s Text: '%s' Line: %d, StartCol: %d, EndCol: %d\n",
         type_str, tok.data, tok.span.line, tok.span.start, tok.span.end);
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
  case TADD:
    name = "+";
    break;
  case TSUB:
    name = "-";
    break;
  case TMUL:
    name = "*";
    break;
  case TDIV:
    name = "/";
    break;
  case TSEMI:
    name = ";";
    break;
  default:
    name = "<unimplemented>";
    break;
  }
  return strdup(name);
}
