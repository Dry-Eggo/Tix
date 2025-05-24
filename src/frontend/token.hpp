#pragma once

#include <string>
enum TokenKind {
  STRINGLIT,
  NUMBER,
  OPERATOR,
  SEPARATOR,
  TXEOF,
  CHAR,
  KEYWORD,
  IDENT,
};

struct Token {
  TokenKind kind;
  std::string data;
  bool eq(Token t) { return t.data == data && t.kind == kind; }
};

inline Token separator(const char *s) {
  return {.kind = TokenKind::SEPARATOR, .data = s};
}

inline Token op(const char *o) { return {TokenKind::OPERATOR, o}; }
inline Token keyword(const char *k) { return {TokenKind::KEYWORD, k}; }
