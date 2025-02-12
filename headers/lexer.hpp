#pragma once

#include "_glob_symbols.hpp"
#include "asmgen.hpp"
#include "eggoLog.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

class Lexer {
public:
  bool _check_main, _is_header;
  NodeProg _prog;
  Parser h_parser;
  inline Lexer(const char *filePath, bool _check_main = true,
               bool is_header = false)
      : m_srcPath(std::move(filePath)), _check_main(_check_main),
        _is_header(is_header) {
    fs::path p = m_srcPath;
    if (!fs::exists(fs::absolute(p))) {
      printf("No Such File or Directory : %s", p.c_str());
      exit(1);
    }

    std::ifstream file(p);
    if (!file.is_open()) {
      printf("Error : Unable to open file");
      exit(0);
    }

    std::stringstream ss;
    ss << file.rdbuf();
    m_srcFile = ss.str();

    file.close();
    auto tokens = lex();

    h_parser.start(tokens, filePath, _check_main);
  }
  inline ~Lexer() {}

private:
  std::string m_srcFile;
  std::string m_srcPath;

  size_t m_index = 0;
  int curline = 1;
  int column = 1;

  inline std::optional<char> peek(int i = 0) {
    if ((m_index + i) >= m_srcFile.length()) {
      return {};
    } else {
      return m_srcFile.at(m_index + i);
    }
  }

  inline char consume() { return m_srcFile.at(m_index++); }

  inline std::vector<Token> lex() {
    std::string buf;
    std::vector<Token> tokens;

    while (peek().has_value()) {
      if (std::isalpha(peek().value())) {
        buf.push_back(consume());
        column++;
        while (peek().has_value() && isalnum(peek().value()) ||
               peek().value() == '_') {
          buf.push_back(consume());
          column++;
        }

        if (buf == "mk") {
          tokens.push_back(
              {.type = TokenType::MK, .line = curline, .col = column});
        }

        else if (buf == "ret") {
          tokens.push_back(
              {.type = TokenType::RET, .line = curline, .col = column});
        }

        else if (buf == "str" || buf == "int" || buf == "bool") {
          tokens.push_back({.value = buf,
                            .type = TokenType::TYPE,
                            .line = curline,
                            .col = column});
        }

        else if (buf == "for") {
          tokens.push_back(
              {.type = TokenType::FOR, .line = curline, .col = column});
        } else if (buf == "mkf") {
          tokens.push_back(
              {.type = TokenType::FUNC, .line = curline, .col = column});
        } else if (buf == "call") {
          tokens.push_back(
              {.type = TokenType::CALL, .line = curline, .col = column});
        } else if (buf == "extern") {
          tokens.push_back(
              {.type = TokenType::EXTERN, .line = curline, .col = column});
        } else if (buf == "while") {
          tokens.push_back(
              {.type = TokenType::WHILE, .line = curline, .col = column});
        } else if (buf == "if") {
          tokens.push_back(
              {.type = TokenType::IF, .line = curline, .col = column});
        } else if (buf == "else") {
          tokens.push_back({.type = ELSE, .line = curline, .col = column});
        } else if (buf == "elif") {
          tokens.push_back({.type = ELIF, .line = curline, .col = column});
        } else if (buf == "true") {
          tokens.push_back(
              {.value = "1", .type = INT_LIT, .line = curline, .col = column});
        } else if (buf == "false") {

          tokens.push_back(
              {.value = "0", .type = INT_LIT, .line = curline, .col = column});
        } else if (buf == "bundle") {
          tokens.push_back(
              {.value = "0", .type = INCLUDE, .line = curline, .col = column});
        } else if (buf == "as") {
          tokens.push_back(
              {.value = "0", .type = AS, .line = curline, .col = column});

        }

        else {
          tokens.push_back({.value = buf,
                            .type = TokenType::IDENT,
                            .line = curline,
                            .col = column});
        }

        buf.clear();
      }

      if (peek().value() == '\"') {
        buf.push_back(consume());
        column++;
        // "Hello"
        while (peek().has_value() && peek().value() != '\"') {
          buf.push_back(consume());
          column++;
          // if(peek().has_value() && peek().value() == '\"')
          //{
          // buf.push_back('\"');
          //}
        }

        if (peek().value() == '\"') {
          buf.push_back(consume());
          column++;
        }

        tokens.push_back({.value = buf,
                          .type = TokenType::STRING_LIT,
                          .line = curline,
                          .col = column});
        buf.clear();
      }

      if (peek().value() == '#') {
        consume();
        if (peek().value() == '!') {
          consume();
          while (peek().value() != '\n') {
            consume();
          }
        } else {
          while (peek().value() != '!') {
            consume();
            if (peek().value() == '\n') {
              curline++;
              column = 1;
            }
          }
          column++;
          consume();
        }
      }

      if (isdigit(peek().value())) {
        column++;
        buf.push_back(consume());
        // Logger::Info("%s", buf.c_str());
        while (peek().has_value() && isdigit(peek().value())) {
          buf.push_back(consume());
          column++;
        }
        tokens.push_back({.value = buf,
                          .type = TokenType::INT_LIT,
                          .line = curline,
                          .col = column});
        buf.clear();
      }

      if (peek().value() == '(') {
        consume();
        tokens.push_back(
            {.type = TokenType::OPAREN, .line = curline, .col = column});
      }

      if (peek().value() == ':') {
        consume();
        if (peek().has_value()) {
          if (peek().value() == ':') {
            consume();
            tokens.push_back(
                {.type = TokenType::SCOPE_RES, .line = curline, .col = column});
          } else {

            tokens.push_back(
                {.type = TokenType::TYPE_DEC, .line = curline, .col = column});
          }
        }
      }

      if (peek().value() == ')') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::CPAREN, .line = curline, .col = column});
      }
      if (peek().value() == ';') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::SEMI, .line = curline, .col = column});
        // printf("Found semi at line : %d\n", curline);
      }
      if (peek().value() == '+') {
        if (peek(1).has_value() && peek(1).value() == '=') {
          consume();
          consume();
          column += 2;
          tokens.push_back(
              {.type = TokenType::ADD_EQU, .line = curline, .col = column});
        } else {
          consume();
          column++;
          tokens.push_back(
              {.type = TokenType::ADD, .line = curline, .col = column});
        }
      }

      if (peek().value() == '%') {
        tokens.push_back(
            {.type = TokenType::R_PTR, .line = curline, .col = column});
        consume();
        column++;
      }

      if (peek().value() == '!' && peek(1).has_value() &&
          peek(1).value() == '=') {
        consume();
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::N_EQU, .line = curline, .col = column});
      }

      if (peek().value() == '>') {
        if (peek(1).has_value() && peek(1).value() == '=') {
          tokens.push_back(
              {.type = TokenType::GTH_EQU, .line = curline, .col = column});

          consume();
          consume();
          column += 2;
        } else {
          consume();
          tokens.push_back(
              {.type = TokenType::GTH, .line = curline, .col = column});
          column++;
        }
      }
      if (peek().value() == '<') {
        if (peek(1).has_value() && peek(1).value() == '=') {
          tokens.push_back(
              {.type = TokenType::LTH_EQU, .line = curline, .col = column});

          consume();
          consume();
          column += 2;
        } else {
          tokens.push_back(
              {.type = TokenType::LTH, .line = curline, .col = column});
          consume();
          column++;
        }
      }
      if (peek().value() == '-') {
        if (peek(1).has_value() && peek(1).value() == '=') {
          tokens.push_back(
              {.type = TokenType::SUB_EQU, .line = curline, .col = column});

          consume();
          consume();
          column += 2;
        } else {
          tokens.push_back(
              {.type = TokenType::SUB, .line = curline, .col = column});
          consume();
          column++;
        }
      }

      if (peek().has_value() && peek().value() == '{') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::OBRACE, .line = curline, .col = column});
      }
      if (peek().value() == '}') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::CBRACE, .line = curline, .col = column});
      }

      if (peek().value() == '=') {
        consume();
        if (peek().has_value() && peek().value() == '=') {
          consume();
          column += 2;
          tokens.push_back(
              {.type = TokenType::EQU, .line = curline, .col = column});
        } else {
          column++;
          tokens.push_back(
              {.type = TokenType::ASSIGN, .line = curline, .col = column});
        }
      }

      if (peek().value() == ',') {
        consume();
        column++;
      }
      if (isspace(peek().value())) {
        if (peek().has_value() && peek().value() == '\n') {
          curline++;
          column = 1;
        }
        consume();
        column++;
      }
      // if(peek().value() == '+')
      //{
      //  consume();
      // tokens.push_back({.type =  TokenType::ADD, .line  curline, .col =
      // column});
      //}

      if (peek().has_value() && peek().value() == '*') {
        consume();
        column++;

        if (peek().has_value() && peek().value() == '=') {
          consume();
          tokens.push_back({.type = MUL_EQU, .line = curline, .col = column});
        } else
          tokens.push_back(
              {.type = TokenType::MUL, .line = curline, .col = column});
      }
      if (peek().has_value() && peek().value() == '/') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::DIV, .line = curline, .col = column});
      }
      /*if ((m_index - column) >= 1 && m_index != 0) {*/
      /*  // we have moved a column*/
      /*  column += m_index - column;*/
      /*}*/
    }
    tokens.push_back({.type = TokenType::eof, .line = curline, .col = column});
    column++;
    Logger::Trace("Token Size : %d", tokens.size());
    for (int i = 0; i < tokens.size(); i++) {
      auto _token = tokens.at(i);
      if (_token.type == TokenType::INCLUDE) {
        auto _header = tokens.at(i + 1);
        _header.value.value().erase(0, 1);
        _header.value.value().erase(_header.value.value().size() - 1, 1);
        std::string _file_path = "nullfile::FNF";
        for (auto _path : search_paths) {
          if (fs::exists(fs::path(_path) / _header.value.value())) {
            printf("%s\n", _path.c_str());
            _file_path = fs::path(_path) / _header.value.value();
            break;
          }
        }
        if (_file_path == "nullfile::FNF") {
          printf("%s does not exist\n", _header.value.value().c_str());
          exit(1);
        }
        Lexer _lex(_file_path.c_str(), false, true);
        _file_path.insert(_file_path.begin(), '\"');
        _file_path.push_back('\"');
        tokens.at(i + 1).value.value() = _file_path;
        for (auto &f : _lex.h_parser.gen.funcStack) {
          if (tokens.at(i + 3).value.has_value())
            extern_funcStack[tokens.at(i + 3).value.value()].push_back(f);
        }
        for (auto &item :
             _lex.h_parser.gen
                 .varScopes[_lex.h_parser.gen.scope_stack.back()]) {
          if (tokens.at(i + 3).value.has_value()) {
            extern_varScopes[tokens.at(i + 3).value.value()].push_back(item);
            extrn_namespaces[tokens.at(i + 3).value.value()].push_back(item);
          }
        }
      }
    }
    return tokens;
  }
};
