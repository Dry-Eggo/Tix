#ifndef TIX_LEXER
#define TIX_LEXER

#define TIX_MAX_STREAM_SIZE 4096
#include <stdio.h>

enum TokenKind {
  TADD,
  TADDADD,
  TSUB,
  TSUBSUB,
  TMUL,
  TDIV,
  TI32,
  TI8,
  TI16,
  TI64,
  TU8,
  TU16,
  TU32,
  TU64,
  TVOID,
  TEOF,
  TNUMBER,
  TSTRING,
  TFN,
  TINT,
  TIF,
  TELSE,
  TELIF,
  TSTR,
  TRETURN,
  TCH,
  TBOOL,
  TEXTRN,
  TOPAREN,
  TCPAREN,
  TOBRACE,
  TCBRACE,
  TMUT,
  TCONST,
  TEQ,
  TSEMI,
  TCOL,
  TIDENT,
  TLET,
  TCOMMA,
  TDOC,
  TMULC, // multi line comments
};

typedef struct {
  int start;
  int end;
  int line;
} Span;

typedef struct {
  int line;
  Span span;
  enum TokenKind kind;
  const char *data;
} Token;

typedef struct TLexer {
  FILE *source_file;
  char *stream;
  int line;
  int col;
  int stream_size;
  int stream_cap;
  char current_char;
  int pos;
} TLexer;

/* initializes a lexer. returns 0 if filenot found 1 is successful*/
int tix_lexer_init(TLexer *lexer, const char *filename);
/* gets the next token in a lexer */
Token tix_lexer_next_token(TLexer *lexer);
/* deallocates a lexer */
void tix_lexer_free(TLexer *);
/* debug prints a token */
void print_token(Token token);
/* frees a token */
void tix_token_free(Token *);
const char *token_tostr(enum TokenKind);

#endif
