#pragma once

#include "../tix.hpp"
#include "token.hpp"
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
struct TixLexer {
  TixCompiler *process;
  size_t current = 0;
  size_t max = 0;
  std::string stream = "";
  int line = 1;
  int col = 1;
  TixLexer(TixCompiler *process) : process(process) {
    if (process->input_path.empty()) {
      /*process->printer->add_substage("Tix Error: No Input path was given");*/
      exit(1);
    } else if (!fs::exists(process->input_path)) {
      std::printf("Tix Error: Input path does not exist\n");
      exit(1);
    }
    auto f = std::ifstream(this->process->input_path);
    std::stringstream s;
    s << f.rdbuf();
    this->stream = s.str();
    this->max = this->stream.length() - 1;
    auto tokens = this->lex();
    tokens.push_back({.kind = TokenKind::TXEOF});
    process->tokens = tokens;
  }
  std::optional<char> peek(int i = 0) {
    if (this->max <= this->current + i) {
      return {};
    }
    return this->stream.at(this->current + i);
  }
  char advance() {
    auto c = this->stream.at(this->current++);
    if (c == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
    return c;
  }
  void skipws() {
    while (this->peek().has_value() && std::isspace(this->peek().value())) {
      this->advance();
    }
  }

  int span_startl;
  int span_startc;
  int span_starti;
  void startSpan() {
    span_startl = line;
    span_startc = col;
    span_starti = current;
  }
  Span endSpan() {
    return {
        SourceLocation(process->input_path, span_startl, span_startc),
        SourceLocation(process->input_path, line, col),
    };
  }
  std::vector<Token> lex() {
    std::vector<Token> tokens;
    while (this->peek().has_value()) {
      this->skipws();
      if (std::isalpha(this->peek().value()) || this->peek().value() == '_') {
        startSpan();
        std::string buf;
        while (this->peek().has_value() &&
               (std::isalnum(this->peek().value()) ||
                this->peek().value() == '_')) {
          buf.push_back(this->advance());
        }
        auto end = endSpan();
        if (buf == "fn" || buf == "extern" || buf == "return" || buf == "mut" ||
            buf == "const" || buf == "i32" || buf == "i8" || buf == "i16" ||
            buf == "i64" || buf == "u8" || buf == "u16" || buf == "u32" ||
            buf == "u64" || buf == "char" || buf == "str") {
          tokens.push_back(
              {.kind = TokenKind::KEYWORD, .data = buf, .span = end});
        } else {
          tokens.push_back(
              {.kind = TokenKind::IDENT, .data = buf, .span = end});
        }
      } else if (std::isdigit(peek().value())) {
        startSpan();
        std::string buf;
        while (peek().has_value() && std::isdigit(peek().value())) {
          buf += advance();
        }
        auto end = endSpan();
        tokens.push_back({.kind = TokenKind::NUMBER, .data = buf, .span = end});
      }

      else if (std::string("{}|()[]:;,.").find(*this->peek()) !=
               std::string::npos) {
        startSpan();
        tokens.push_back({.kind = TokenKind::SEPARATOR,
                          .data = std::string(1, *this->peek()),
                          .span = endSpan()});
        this->advance();
      } else if (std::string("+-*/=&%<>").find(*this->peek()) !=
                 std::string::npos) {
        startSpan();
        tokens.push_back({.kind = TokenKind::OPERATOR,
                          .data = std::string(1, this->peek().value()),
                          .span = endSpan()});
        this->advance();
      } else if (peek().value() == '\"') {
        startSpan();
        advance();
        std::string buf;
        while (peek().has_value() && *peek() != '\"') {
          buf += advance();
        }
        advance();
        tokens.push_back(
            {.kind = TokenKind::STRINGLIT, .data = buf, .span = endSpan()});
      } else {
        printf("Unexected Character in file stream (%c)\n", peek().value());
        exit(1);
      }
    }
    return tokens;
  }
};
