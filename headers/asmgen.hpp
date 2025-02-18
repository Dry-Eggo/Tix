#pragma once

#include "_glob_symbols.hpp"
#include "eggoLog.hpp"
#include "token.hpp"
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace Semantics {
enum symtype { func, scope };
bool find_symbol(std::vector<Var> scope, Token symbol, symtype type) {
  for (auto &_item : scope) {
    if (_item.name.value.value() == symbol.value.value()) {

      if (type == func) {
        if (_item.is_function != true)
          return false;
      }
      return true;
    }
  }

  return false;
}
void Throw_Error(const char *_format, ...) {
  va_list args;
  va_start(args, _format);
  printf("Error : ");
  vprintf(_format, args);
  printf("\n");
  exit(1);
}
} // namespace Semantics

class AsmGen {
public:
  NodeProg m_program;
  std::fstream out_asm;
  std::vector<Var> varStack;
  std::vector<NodeFuncStmt> funcStack;
  bool main_ret = false;
  std::string cur_nameSpace;

  // ================== Scopes ======================= //
  std::map<std::string, std::vector<Var>> varScopes;
  std::vector<std::string> scope_stack; // to track current scope
  std::vector<std::string> bundles;

  std::vector<std::string> queue;
  size_t currOffset = 1;
  size_t loop_count = 0;
  size_t gbvalcounter = 0;
  std::stringstream TEXT, BSS, DATA, FUNC, HEADER, MAIN;

  inline void push(Var v) {
    TEXT << "\n\tmov rax, " << v.value.value.value() << "\n";
    TEXT << "\tpush rax\n";
    v.stackOffset = currOffset++;
  }

  inline void pop(Var v, const char *reg) {
    if (currOffset - 1 != 0 && v.value.type != TokenType::STRING_LIT)
      TEXT << "\n\tmov " << reg << ", [rbp - " << 8 * v.stackOffset << "]\n";
    else
      TEXT << "\n\tmov " << reg << ", " << v.name.value.value() << "\n";
  }
  void make(std::string name, std::string val, std::string met = "db") {
    DATA << "\n\t" + name + " " + met + " " + val + ", 0";
  }
  void resb(std::string name, char *size) {
    BSS << "\n\t" + name + " resd " + size;
  }
  bool validate(Token t) {

    auto var_name = scope_stack.back() + "." + t.value.value();
    for (auto &_item : varScopes[scope_stack.back()]) {
      if (_item.name.value.value() == var_name) {
        return true;
      }
    }
    return false;
  }
  void move(std::stringstream *str, std::string loc, std::string val,
            bool raw = false) {
    if (!raw)
      *str << "\n\tmov " << loc << ", " << val;
    else
      *str << "\n\tmov " << loc << ", [" << val << "]";
  }

  void str_expr(std::vector<std::string> expr, std::stringstream *p_ss) {
    if (expr.size() == 1)
      return;

    for (int i = 0; i < expr.size(); i++) {
      if (expr.at(i) == "/") {
        auto lhs = expr.at(i - 1);
        auto rhs = expr.at(i + 1);

        if (lhs == "pop") {
          *p_ss << "\n\tpop rcx";
        } else {
          bool is_digit = true;
          for (char c : lhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rax, " << lhs;
          else
            *p_ss << "\n\tmov rax, [" << lhs << "]";
        }
        if (rhs == "pop") {
          *p_ss << "\n\tpop rcx";
        } else {
          bool is_digit = true;
          for (char c : rhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rcx, " << rhs;
          else
            *p_ss << "\n\tmov rcx, [" << rhs << "]";
        }

        *p_ss << "\n\tmov rax, rax";
        *p_ss << "\n\tcqot";
        *p_ss << "\n\tmov rbx, rcx";
        *p_ss << "\n\tidiv rbx";

        expr.at(i) = "pop";

        if (expr.size() != 1) {
          expr.erase(expr.begin() + (i + 1));
          expr.erase(expr.begin() + (i - 1));
        }
      }
    }
    for (int i = 0; i < expr.size(); i++) {
      if (expr.at(i) == "*") {
        auto lhs = expr.at(i - 1);
        auto rhs = expr.at(i + 1);

        if (lhs == "pop") {
          *p_ss << "\n\tpop rax";
        } else {
          bool is_digit = true;
          for (char c : lhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rax, " << lhs;
          else
            *p_ss << "\n\tmov rax, [" << lhs << "]";
        }
        if (rhs == "pop") {
          *p_ss << "\n\tpop rcx";
        } else {
          bool is_digit = true;
          for (char c : rhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rcx, " << rhs;
          else
            *p_ss << "\n\tmov rcx, [" << rhs << "]";
        }

        *p_ss << "\n\timul rax, rcx";

        expr.at(i) = "pop";

        if (expr.size() != 1) {
          expr.erase(expr.begin() + (i + 1));
          expr.erase(expr.begin() + (i - 1));
        }
      }
    }
    for (int i = 0; i < expr.size(); i++) {
      if (expr.at(i) == "+") {
        auto lhs = expr.at(i - 1);
        auto rhs = expr.at(i + 1);

        if (lhs == "pop") {
          *p_ss << "\n\tpop rax";
        } else {
          bool is_digit = true;
          for (char c : lhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rax, " << lhs;
          else
            *p_ss << "\n\tmov rax, [" << lhs << "]";
        }
        if (rhs == "pop") {
          *p_ss << "\n\tpop rcx";
        } else {
          bool is_digit = true;
          for (char c : rhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rcx, " << rhs;
          else
            *p_ss << "\n\tmov rcx, [" << rhs << "]";
        }

        *p_ss << "\n\tadd rax, rcx";

        expr.at(i) = "pop";

        if (expr.size() != 1) {
          expr.erase(expr.begin() + (i + 1));
          expr.erase(expr.begin() + (i - 1));
        }
      }
    }
    for (int i = 0; i < expr.size(); i++) {
      if (expr.at(i) == "-") {
        auto lhs = expr.at(i - 1);
        auto rhs = expr.at(i + 1);

        if (lhs == "pop") {
          *p_ss << "\n\tpop rax";
        } else {
          bool is_digit = true;
          for (char c : lhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rax, " << lhs;
          else
            *p_ss << "\n\tmov rax, [" << lhs << "]";
        }
        if (rhs == "pop") {
          *p_ss << "\n\tpop rcx";
        } else {
          bool is_digit = true;
          for (char c : rhs) {
            if (!std::isdigit(c)) {
              is_digit = false;
              break;
            }
          }

          if (is_digit)
            *p_ss << "\n\tmov rcx, " << rhs;
          else
            *p_ss << "\n\tmov rcx, [" << rhs << "]";
        }

        *p_ss << "\n\tsub rax, rcx";

        expr.at(i) = "pop";

        if (expr.size() != 1) {
          expr.erase(expr.begin() + (i + 1));
          expr.erase(expr.begin() + (i - 1));
        }
      }
    }

    if (expr.size() > 1) {
      str_expr(expr, p_ss);
    }
  }

  void gen_expr(std::shared_ptr<std::vector<NodeBinaryExpr>> b,
                std::stringstream *p_ss) {

    std::vector<std::string> expression;

    struct exprVisitor {
      AsmGen *gen;
      std::stringstream *p_ss;
      std::vector<std::string> *expr;
      void operator()(NodeInt i) {
        if (i.value.value.has_value())
          expr->push_back(i.value.value.value());
      }

      void operator()(NodeFuncCall f) {
        static int i = 0;
        if (f.identifier.value.has_value()) {
          std::vector<NodeStmts> s;
          NodeStmts st;
          st.var = f;
          s.push_back(st);
          gen->generate(s, *p_ss, *p_ss,
                        gen->varScopes[gen->scope_stack.back()]);
          *p_ss << "\n\tpush rax";
          expr->push_back("pop");
        }
      }
      void operator()(Token t) {
        std::string var_name = gen->scope_stack.back() + "." + t.value.value();
        for (auto var : gen->varScopes[gen->scope_stack.back()]) {
          if (var.name.value.value() == var_name) {
            expr->push_back(var_name);
          }
        }
      }
    };
    for (int i = 0; i < b->size(); i++) {

      std::visit(exprVisitor{this, p_ss, &expression}, b->at(i).lhs);

      expression.push_back(b->at(i).op);

      std::visit(exprVisitor{this, p_ss, &expression}, b->at(i).rhs);
    }

    expression.erase(std::remove(expression.begin(), expression.end(), ""),
                     expression.end());

    for (int i = 0; i < expression.size(); i++) {
    }
    str_expr(expression, p_ss);
  }
  std::string _path;
  bool _check_main;
  inline AsmGen() {}
  inline void start(NodeProg program, std::string _path,
                    bool _check_main = true) {
    _Tix_current_file = fs::absolute(fs::path(_path)).filename().string();
    cur_nameSpace = _Tix_current_file;
    this->_path = _path;
    this->_check_main = _check_main;
    varScopes["main"] = varStack;
    scope_stack.push_back("main");

    BSS << "\nsection .bss\n";
    DATA << "\nsection .data\n";

    std::filesystem::path out_path = _path + ".asm";
    out_asm.open(out_path, std::ios::out);

    HEADER << "\nsection .text\n";
    if (_check_main)
      HEADER << "global _start\n";
    HEADER << "\n\textern std_terminate_process";
    if (_check_main)
      HEADER << "\n_start:\n";
    if (_check_main)
      HEADER << "\n\n\tmov rbp, rsp\n\tcall main\n";

    Logger::Trace("Generating Asm");
    Logger::Trace("Program Size : %d", m_program.stmt.size());

    generate(program.stmt, TEXT, TEXT, varStack);

    bool is_good = false;

    for (auto f : funcStack) {
      if (f.identifier.value.value() == "main") {
        is_good = true;
        break;
      }
    }
    if (!_check_main) {
      is_good = true;
    }

    if (!is_good) {
      printf("\nFatal : Unable to identify main entry point with label : "
             "main\nterminated with failure\n");
      exit(1);
    }

    out_asm << BSS.str();
    out_asm << DATA.str();
    out_asm << HEADER.str();
    out_asm << TEXT.str();

    for (auto &p : queue) {
      FUNC << p;
    }

    out_asm << FUNC.str();
    out_asm << MAIN.str();
    if (!main_ret)
      out_asm << "\n\tmov rdi, 0";

    if (_check_main) {
      out_asm << "\n\tcall std_terminate_process\n";
      out_asm << "\nsection .note.GNU-stack";
    }
  }

  inline void changeScope(std::string name) {
    scope_stack.push_back(name);
    if (varScopes.find(name) == varScopes.end()) {
      varScopes[name] = {};
    }
  }

  inline void exitScope() {
    if (scope_stack.empty())
      return;

    std::string cur_scope = scope_stack.back();
    scope_stack.erase(
        std::remove(scope_stack.begin(), scope_stack.end(), scope_stack.back()),
        scope_stack.end());
    varScopes.erase(cur_scope);
  }

  inline void generate(std::vector<NodeStmts> stmts, std::stringstream &n_s,
                       std::stringstream &p_s, std::vector<Var> &curScope) {
    for (auto stmt : stmts) {
      struct stmtVisitor {
        AsmGen *gen;
        std::vector<Var> *curScope = &gen->varScopes[gen->scope_stack.back()];
        std::stringstream *c_ss;
        std::stringstream *p_ss;

        void operator()(NodeExitStmt e) {
          /* Out of commision for Now */

          /* Replaced With std_terminate_process */

          /*   void operator()(NodeInt i) { */
          /*     gen->TEXT << "\n\tmov rdi, " << i.value.value.value() <<
           * "\n";
           */
          /*     gen->TEXT << "\tcall std_terminate_process\n"; */
          /*   } */
          /*   void operator()(NodeBinaryExpr i) {} */
          /* }; */
          /* std::visit(exprVisitor{.gen = gen}, e.expr.var); */

          /* Logger::Info("Generating Exit statement"); */
        }

        void operator()(NodeMkStmt m) {
          struct mkVisitor {
            Token *ident;
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeMkStmt pm;
            std::string var_name =
                gen->scope_stack.back() + "." + pm.identifier.value.value();

            void operator()(NodeString s) {

              if (pm.type != DataType::STR) {
                printf("Fatal : Cannot pass value of type "
                       "\"str\" to \"%s\"\n",
                       pm.identifier.value.value().c_str());
                exit(1);
              }
              gen->DATA << "\n\t" << var_name << " db " << s.value.value.value()
                        << " ,0";
              Var v;
              v.name = {.value = var_name};
              v.value = {.value = s.value.value.value()};
              v.type = STR;
              v.stackOffset = gen->currOffset++;
              v.is_initialized = true;
              *p_ss << "\n\tmov qword [rbp + " << v.stackOffset * 8 << "], "
                    << var_name;
              gen->varScopes[gen->scope_stack.back()].push_back(v);
            }

            void operator()(NodeInt i) {

              if (pm.type != DataType::INT) {
                if (pm.type != DataType::BOOL) {
                  printf("Fatal : Cannot pass value of type "
                         "\"int\" to \"%s\"\n",
                         pm.identifier.value.value().c_str());
                  exit(1);
                }
              }
              Var v;
              v.name = {.value = var_name};
              v.value = {.value = i.value.value.value()};
              v.type = INT;
              v.stackOffset = gen->currOffset++;
              v.is_initialized = true;
              *p_ss << "\n\tmov qword [rbp + " << v.stackOffset * 8 << "], "
                    << i.value.value.value();
              gen->varScopes[gen->scope_stack.back()].push_back(v);
            }
            void
            operator()(const std::shared_ptr<std::vector<NodeBinaryExpr>> &b) {
              gen->resb(var_name, (char *)"1");
              gen->gen_expr(b, p_ss);
              *p_ss << "\n\tpop rax";
              *p_ss << "\n\tmov [" + var_name + "], rax";
              gen->varScopes[gen->scope_stack.back()].push_back(
                  {.name = {.value = var_name}, .value = {.value = "rax"}});
            }
            void operator()(const std::shared_ptr<NodeFuncCall> &s) {
              /*gen->BSS << "\n\t" << var_name << " resb 1024";*/
              std::vector<NodeStmts> stmt;
              stmt.push_back({.var = *s});
              gen->generate(stmt, *p_ss, *p_ss,
                            gen->varScopes[gen->scope_stack.back()]);
              Var var;
              var.name = {.value = var_name};
              var.stackOffset = gen->currOffset++;
              var.type = pm.type;
              var.is_initialized = true;
              /**p_ss << "\n\tpop rax";*/
              *p_ss << "\n\tmov qword [rbp + " << var.stackOffset * 8
                    << "], rax";
              gen->varScopes[gen->scope_stack.back()].push_back(var);
            }
            void operator()(NodeCmp &c) {}
            void operator()(Token t) {
              // reserving space for the variable
              gen->resb(var_name, (char *)"1024");
              // check if the value beign passed is valid
              auto _newValue = gen->scope_stack.back() + "." + t.value.value();
              bool found = false;
              Var _tester;
              for (auto &_var : gen->varScopes[gen->scope_stack.back()]) {
                if (_var.name.value.value() == _newValue) {
                  found = true;
                  _tester = _var;
                }
              }
              if (found) {
                // type checking
                if (_tester.type != pm.type) {
                  printf("Cannot pass value of \"%s\" to \"%s\" :: Invalid "
                         "Type Matching\n",
                         t.value.value().c_str(),
                         pm.identifier.value.value().c_str());
                  exit(1);
                }
                /**p_ss << "\n\tmov rax, ["*/
                /*      << gen->scope_stack.back() + "." + t.value.value() <<
                 * "]";*/
                /**p_ss << "\n\tmov [" << var_name << "], rax";*/
                *p_ss << "\n\tmov rax, qword [rbp + " << _tester.stackOffset * 8
                      << "]";
                *p_ss << "\n\tmov qword [rbp + " << gen->currOffset * 8
                      << "], rax";
              } else {
                printf("Error : \"%s\" is not declared in this scope\n",
                       t.value.value().c_str());
                exit(1);
              }
              Var v;
              v.name = {.value = var_name};
              v.value = {.value = t.value.value()};
              v.type = pm.type;
              v.stackOffset = gen->currOffset++;
              v.is_initialized = true;
              gen->varScopes[gen->scope_stack.back()].push_back(v);
            }
          };
          if (m.is_initialized)
            std::visit(mkVisitor{&m.identifier, gen, p_ss, m}, m.value->var);
          else {
            *p_ss << "\n\tsub rsp, 1";
            *p_ss << "\n\tmov qword [rbp +" << gen->currOffset * 8 << "], rsp";
            gen->currOffset++;
          }
        }

        void operator()(NodeReStmt r) {
          bool found = false;

          // check if variable has been declared before

          struct mkVisitor {
            Token *ident;
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeReStmt pm;
            std::string var_name =
                gen->scope_stack.back() + "." + pm.identifier.value.value();

            void operator()(NodeString s) {

              Var var;
              std::string gbval = "gbval" + std::to_string(gen->gbvalcounter++);

              for (auto &_item : gen->varScopes[gen->scope_stack.back()]) {
                if (_item.name.value == var_name) {
                  var = _item;
                }
              }
              gen->make(gbval, s.value.value.value());
              *p_ss << "\n\tmov qword [rbp + " << var.stackOffset * 8 << "], "
                    << gbval;
            }

            void operator()(NodeInt i) {
              Var var;

              for (auto &_item : gen->varScopes[gen->scope_stack.back()]) {
                if (_item.name.value == var_name) {
                  var = _item;
                }
              }
              *p_ss << "\n\tmov qword [rbp + " << var.stackOffset * 8 << "], "
                    << i.value.value.value();
            }
            void
            operator()(const std::shared_ptr<std::vector<NodeBinaryExpr>> &b) {
              gen->gen_expr(b, p_ss);
              *p_ss << "\n\tpop rax";
              *p_ss << "\n\tmov [" + var_name + "], rax";
            }
            void operator()(const std::shared_ptr<NodeFuncCall> &s) {}
            void operator()(NodeCmp &c) {}
            void operator()(Token t) {
              Var var;
              Var nvar;
              auto nvar_name = gen->scope_stack.back() + t.value.value();
              for (auto v : gen->varScopes[gen->scope_stack.back()]) {
                if (v.name.value == var_name) {
                  var = v;
                }
                if (v.name.value == nvar_name) {
                  nvar = v;
                }
              }
              *p_ss << "\n\tmov rax, qword [rbp + " << nvar.stackOffset * 8
                    << "]";
              *p_ss << "\n\tmov qword [rbp + " << var.stackOffset * 8
                    << "], rax";
            }
          };

          std::visit(mkVisitor{&r.identifier, gen, p_ss, r}, r.new_value->var);

          // mov new-value into the address of variable
        }

        void operator()(NodeForStmt f) {

          //  initialize the start value i.e     counter resd 1
          //									   mov
          // dword [counter], 0

          std::string for_name = "for" + std::to_string(gen->loop_count++);
          auto ident_name = for_name + "." + f.identifier.value.value();
          std::stringstream f_header;

          gen->resb(for_name + "." + f.identifier.value.value(), (char *)"1");

          *c_ss << "\n\tmov dword [" + ident_name + "], "
                << f.startValue.value.value();

          // saving registers

          *p_ss << "\n\tpush rax\n\tpush r9\n\tpush r8\n";

          *p_ss << "\n" + for_name + ":";

          //  check comparisons

          struct forVisit {
            AsmGen *gen;
            std::stringstream *p_ss;
            std::string ident_name;
            bool rhs = false;

            void operator()(const std::shared_ptr<NodeFuncCall> &f) {}
            void operator()(NodeInt i) {
              *p_ss << "\n\tmov ax, " << i.value.value.value();
            }
            void operator()(Token t) {
              bool found_token = false;
              std::string var_name;
              for (int i = 0; i < gen->scope_stack.size(); i++) {
                var_name = gen->scope_stack.at(i) + "." + t.value.value();
                if (i > gen->scope_stack.size()) {
                  printf("Use of Undeclared Identifier \"%s\"",
                         var_name.c_str());
                  // exit(1);
                } else {
                  for (auto v : gen->varScopes[gen->scope_stack.at(i)]) {
                    if (var_name == v.name.value.value()) {
                      found_token = true;
                      break;
                    }
                  }
                  if (found_token)
                    break;
                }
                if (found_token)
                  break;
              }
              // i move the value at the address of the variable into eax
              // moving the label will move the memory address which is
              // different from the actual value
              if (rhs) {
                *p_ss << "\n\tmov ax, [" + var_name + "]";
              } else
                *p_ss << "\n\tmov cx, [" + var_name + "]";
            }
          };
          gen->changeScope(for_name);
          Var v;
          v.name = {.value = ident_name};
          v.value = f.startValue;
          if (f.startValue.type == STRING_LIT)
            v.type = DataType::STR;
          else if (f.startValue.type == INT_LIT)
            v.type = DataType::INT;
          gen->varScopes[gen->scope_stack.back()].push_back(v);
          // gen->varScopes[gen->scope_stack.back()].push_back({.name =
          // {.value = ident_name}, .value =f.startValue, .type =
          // f.startValue.type});

          std::visit(forVisit{gen, p_ss, ident_name}, f.condition->lhs);
          std::visit(forVisit{gen, p_ss, ident_name, true}, f.condition->rhs);

          *p_ss << "\n\tcmp rcx, rax";
          if (f.condition->cmp_s == "==")
            *p_ss << "\n\tjne " << for_name + "end";
          else if (f.condition->cmp_s == "!=")
            *p_ss << "\n\tje " << for_name + "end";
          else if (f.condition->cmp_s == ">")
            *p_ss << "\n\tjle " << for_name + "end";
          else if (f.condition->cmp_s == "<")
            *p_ss << "\n\tjge " << for_name + "end";
          else if (f.condition->cmp_s == ">=")
            *p_ss << "\n\tjl " << for_name + "end";
          else if (f.condition->cmp_s == "<=")
            *p_ss << "\n\tjg " << for_name + "end";

          // generate body

          gen->generate(f.body, *c_ss, *p_ss,
                        gen->varScopes[gen->scope_stack.back()]);

          struct incVisit {

            AsmGen *gen;
            std::stringstream *p_ss;
            std::string ident_name;

            void operator()(const std::shared_ptr<NodeFuncCall> &f) {}
            void operator()(NodeInt i) {
              *p_ss << "\n\tmov eax, " + i.value.value.value();
            }
            void operator()(Token t) {
              bool found_token = false;
              std::string var_name;
              for (int i = 0; i < gen->scope_stack.size(); i++) {
                var_name = gen->scope_stack.at(i) + "." + t.value.value();
                if (i > gen->scope_stack.size()) {
                  printf("Use of Undeclared Identifier \"%s\"",
                         var_name.c_str());
                  exit(1);
                } else {
                  for (auto v : gen->varScopes[gen->scope_stack.at(i)]) {
                    if (var_name == v.name.value.value()) {
                      found_token = true;
                      break;
                    }
                  }
                  if (found_token)
                    break;
                }
                if (found_token)
                  break;
              }
              // i move the value at the address of the variable into eax
              // moving the label will move the memory address which is
              // different from the actual value
              *p_ss << "\n\tmov eax, [" + var_name + "]";
            }
          };

          std::visit(incVisit{gen, p_ss, ident_name}, f.increment.rhs);
          if (f.increment.cmp_s == "+=")
            *p_ss << "\n\tadd dword [" + ident_name + "], eax\n\tjmp " +
                         for_name + "\nnop";
          else if (f.increment.cmp_s == "-=")
            *p_ss << "\n\tsub dword [" + ident_name + "], eax\n\tjmp " +
                         for_name;

          *p_ss << "\n" + for_name + "end:";
          *p_ss << "\n\tpop r9\n\tpop r9\n\tpop rax\n";

          gen->exitScope();
        }
        void operator()(NodeFuncStmt f) {

          std::vector<Var> scope;
          printf("Generating %s\n", f.identifier.value.value().c_str());
          std::stringstream temp;
          std::stringstream nest;
          // Generating mangled_name
          auto func_name =
              "_Tix_" + gen->cur_nameSpace + "_" + f.identifier.value.value();
          for (auto p : f.params) {
            if (p.type == DataType::STR)
              func_name += "_str";
            else if (p.type == DataType::INT)
              func_name += "_int";
          }
          func_name += "E";
          f.mangled_name = func_name;

          size_t scope_offset = 1; // cover the memory of the current scope;

          gen->funcStack.push_back(f);
          Var function;
          function.is_function = true;
          function.name = f.identifier;
          function.type = f.ret_type;
          gen->varScopes[gen->scope_stack.back()].push_back(function);
          gen->changeScope(f.identifier.value.value());
          if (f.identifier.value.value() != "main")
            temp << "\n" << f.mangled_name << ":\n";
          else {
            temp << "\nmain:";
          }
          /*temp << "\n\tpush rbp";*/
          /*temp << "\n\tmov rbp, rsp";*/
          // argument name uniqueness with current scope prefix
          std::string param_name;
          if (f.identifier.value.value() == "main")
            func_name = "main";
          for (int i = 0; i < f.param_count; i++) {
            param_name =
                func_name + "." + f.params.at(i).identifier.value.value();
            Var v;
            v.name.value = param_name;
            v.type = f.params.at(i).type;
            gen->varScopes[gen->scope_stack.back()].push_back(v);
            gen->BSS << "\n\t" << param_name << " resb 1024";
            switch (i) {
            case 0:
              temp << "\n\tmov [rbp -8], rdi\n";
              scope.push_back({.name = {.value = param_name},
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              break;
            case 1:
              scope.push_back({.name = {.value = param_name},
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              temp << "\n\tmov [rbp -16], rsi\n";
              break;
            case 2:
              scope.push_back({.name = {.value = param_name},
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              temp << "\n\tmov [rbp -24], rdx\n";
            default:
              break;
            }
            f.params.at(i).identifier.value.value() = param_name.c_str();
          }
          if (f.identifier.value.value() != "main")
            gen->generate(f.body, gen->MAIN, temp,
                          scope); // stream for child statements
          else
            gen->generate(f.body, gen->MAIN, temp, gen->varStack);

          struct _ret_visitor {
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeFuncStmt f;
            void operator()(NodeInt i) {
              if (f.ret_type == DataType::INT)
                *p_ss << "\n\tmov rax, " << i.value.value.value();
            }
            void operator()(std::shared_ptr<std::vector<NodeBinaryExpr>> &b) {
              gen->gen_expr(b, p_ss);
              *p_ss << "\n\tmov rax, rax";
              *p_ss << "\n\tret";
            }
            void operator()(std::shared_ptr<NodeFuncCall> &f) {}
            void operator()(Token t) {
              if (gen->validate(t)) {
                *p_ss << "\n\tmov rax, ["
                      << gen->scope_stack.back() + "." + t.value.value() << "]";
                *p_ss << "\n\tret";
              }
            }
            void operator()(NodeString) {}
            void operator()(NodeCmp c) {}
          };

          gen->main_ret = true;
          /*temp << "\n\tpop rbp\n";*/
          if (f.has_ret)
            std::visit(_ret_visitor{gen, &temp, f}, f.ret_value.value->var);
          /*for (int i = 0; i < f.param_count; i++) {*/
          /*  if (f.params.at(i).type == DataType::STR) {*/
          /*    temp << "\n\tlea rdi, [" << param_name*/
          /*         << "]\n\tmov rsi, 1024\n\tcall std_clear_string\n";*/
          /*  }*/
          /*}*/
          if (f.identifier.value.value() == "main") {
            gen->MAIN << temp.str();
            /*gen->varScopes["main"] = scope;*/
          } else {
            temp << "\n\tret";
            gen->FUNC << temp.str();
            gen->varScopes[f.identifier.value.value()] = scope;
          }

          gen->exitScope();
        }

        void operator()(NodeFuncCall fc) {
          // valid params
          bool _is_external = false;
          bool _is_valid = false;

          // validate that the function exists
          NodeFuncStmt _function_symbol;
          if (fc.is_extrn) {
            for (auto &_item : extern_funcStack[gen->cur_nameSpace]) {
              if (_item.identifier.value.value() ==
                  fc.identifier.value.value()) {
                _function_symbol = _item;
                _is_valid = true;
              }
            }
          }

          if (!_is_valid) {
            if (Semantics::find_symbol(gen->varScopes[gen->scope_stack.back()],
                                       fc.identifier,
                                       Semantics::symtype::func)) {
              _is_valid = true;
              for (auto &_item : gen->funcStack) {
                if (_item.identifier.value.value() ==
                    fc.identifier.value.value()) {
                  _function_symbol = _item;
                  _is_valid = true;
                }
              }
            }
          }

          // pass parameters to arguments
          // call the function
          *p_ss << "\n\tcall " + _function_symbol.mangled_name;
        }

        void operator()(NodeCallStmt c) {
          // validate
          auto name = c.std_lib_value.value.value();
          bool is_valid = false;

          for (auto &_scope : gen->varScopes) {
            if (Semantics::find_symbol(_scope.second, c.std_lib_value,
                                       Semantics::symtype::func)) {
              is_valid = true;
              break;
            }
          }
          if (!is_valid) {
            for (auto &_scope : extern_varScopes) {
              if (Semantics::find_symbol(_scope.second, c.std_lib_value,
                                         Semantics::symtype::func)) {
                is_valid = true;
                break;
              }
            }
            if (!is_valid)
              Semantics::Throw_Error("%s was not declared in this scope",
                                     name.c_str());
          }
          // layout params
          Var v;
          for (auto &param : c.params) {
            std::string param_name;
            if (gen->scope_stack.back() != "main")
              param_name = "_Tix_" + gen->cur_nameSpace + "_" +
                           gen->scope_stack.back() + "." +
                           param.value.value.value();
            else
              param_name = "main." + param.value.value.value();
            param.value.value = param_name;
            bool is_valid_p = false;

            printf("%s was given as parameter\n", param_name.c_str());
            for (auto &_scope : gen->varScopes) {
              if (Semantics::find_symbol(_scope.second, param.value,
                                         Semantics::symtype::scope)) {
                is_valid_p = true;
                for (auto &_item : _scope.second) {
                  if (_item.name.value == param.value.value) {
                    v = _item;
                    param.stackOffset = _item.stackOffset;
                  }
                }
                break;
              }
            }

            // TODO : Check bundled symbols for the parameter
            /*if (!is_valid_p)*/
            /*  Semantics::Throw_Error("'%s' was not declared in this scope",*/
            /*                         param.value.value.value().c_str());*/
          }
          auto param_count = c.params.size();
          for (int i = 0; i < param_count; i++) {
            bool is_ptr = (c.params.at(i).value.is_ptr) ? true : false;
            auto param = c.params.at(i);
            switch (i) {
            case 0:
              *p_ss << "\n\tmov rdi, qword [rbp + " << param.stackOffset * 8
                    << "]";
              break;
            case 1:
              *p_ss << "\n\tmov rsi, [rbp + " << param.stackOffset * 8 << "]";
              break;

            case 2:
              *p_ss << "\n\tmov rdx, [rbp + " << param.stackOffset * 8 << "]";
              break;
            default:
              *p_ss << "\n\tpush [rbp + " << param.stackOffset * 8 << "]";
              break;
            }
          }

          // call
          *p_ss << "\n\tcall " + c.std_lib_value.value.value();
        }
        void operator()(NodeExtrnStmt e) {
          std::vector<Var> scope;
          std::string arg_name;
          Var extrn_var;
          extrn_var.name = e.identifier;
          extrn_var.is_function = true;
          gen->varScopes[gen->scope_stack.back()].push_back(extrn_var);

          for (auto p : e.param) {
            arg_name =
                e.identifier.value.value() + "." + p.identifier.value.value();
            Var v;
            v.name = {.value = arg_name};
            switch (p.type) {
            case INT:
              v.type = INT;
              break;
            case STR:
              v.type = STR;
              break;
            case BOOL:
              v.type = BOOL;
              break;
            default:
              break;
            }
            gen->BSS << "\n\t" << arg_name << " resb 1024";
            scope.push_back(v);
          }
          gen->HEADER << "\n\textern " << e.identifier.value.value();
          gen->varScopes[e.identifier.value.value()] = scope;
        }
        void operator()(NodeWhileStmt w) {
          auto loop_name = "while" + std::to_string(gen->loop_count++);
          std::string lhs;
          std::string rhs;
          // generate condition-
          /*gen->changeScope(loop_name);*/
          struct visitor {
            AsmGen *gen;
            std::stringstream *p_ss;
            bool lhs = true;
            void operator()(const std::shared_ptr<NodeFuncCall> &f) {
              // check for function definition
              bool _func_defined = false;
              for (auto &_func : gen->funcStack) {
                if (_func.identifier.value == f->identifier.value) {
                  _func_defined = true;
                }
              }
              if (_func_defined) {
                *p_ss << "\n\tcall " << f->identifier.value.value();
                if (!lhs)
                  *p_ss << "\n\tmov rax, rax";
                else
                  *p_ss << "\n\tmov rcx, rax";
              } else {
                printf("Error : Use of undeclared identifier \"%s\"\n",
                       f->identifier.value.value().c_str());
                exit(1);
              }
            }
            void operator()(NodeInt i) {
              if (!lhs)
                *p_ss << "\n\tmov cx, " + i.value.value.value();
              else {
                *p_ss << "\n\tmov ax, " + i.value.value.value();
                lhs = false;
              }
            }
            void operator()(Token t) {
              bool found_token = false;
              std::string var_name;
              for (int i = 0; i < gen->scope_stack.size(); i++) {
                var_name = gen->scope_stack.at(i) + "." + t.value.value();
                if (i > gen->scope_stack.size() - 1) {
                  printf("Use of Undeclared Identifier \"%s\"\n",
                         var_name.c_str());
                  // exit(1);
                } else {
                  for (auto v : gen->varScopes[gen->scope_stack.at(i)]) {
                    if (var_name == v.name.value.value()) {
                      found_token = true;
                      break;
                    }
                  }
                  if (found_token)
                    break;
                }
              }
              if (!found_token) {
                printf("Use of Undeclared Identifier \"%s\"\n",
                       var_name.c_str());
                // exit(1);
              }
              // i move the value at the address of the variable into eax
              // moving the label will move the memory address which is
              // different from the actual value
              if (!lhs)
                *p_ss << "\n\tmov cx, [" + var_name + "]";
              else {
                *p_ss << "\n\tmov ax, [" + var_name + "]";
                lhs = false;
              }
            }
          };

          std::visit(visitor{gen, p_ss, true}, w.comparisons.lhs);
          if (!w.comparisons.is_single_condition) {
            std::visit(visitor{gen, p_ss, false}, w.comparisons.rhs);
          }
          *p_ss << "\n\tcmp ax, cx";

          if (w.comparisons.cmp_s == "==") {
            *p_ss << "\n\tje " << loop_name;
            *p_ss << "\n\tjne " << loop_name << "end";
          } else if (w.comparisons.cmp_s == ">=") {
            *p_ss << "\n\tjge " << loop_name;
            *p_ss << "\n\tjl " << loop_name << "end";
          } else if (w.comparisons.cmp_s == "<=") {
            *p_ss << "\n\tjle " << loop_name;
            *p_ss << "\n\tjg " << loop_name << "end";
          } else if (w.comparisons.cmp_s == ">") {
            *p_ss << "\n\tjg " << loop_name;
            *p_ss << "\n\tjle " << loop_name << "end";
          } else if (w.comparisons.cmp_s == "<") {
            *p_ss << "\n\tjl " << loop_name;
            *p_ss << "\n\tjge " << loop_name << "end";
          } else if (w.comparisons.cmp_s == "!=") {
            *p_ss << "\n\tjne " << loop_name;
            *p_ss << "\n\tje " << loop_name << "end";
          }

          // generate body-
          *p_ss << "\n" << loop_name << ":";
          gen->generate(w.body, *p_ss, *p_ss,
                        gen->varScopes[gen->scope_stack.back()]);
          std::visit(visitor{gen, p_ss, true}, w.comparisons.lhs);
          if (!w.comparisons.is_single_condition) {
            std::visit(visitor{gen, p_ss, false}, w.comparisons.rhs);
          }
          *p_ss << "\n\tcmp ax, cx";

          if (w.comparisons.cmp_s == "==") {
            *p_ss << "\n\tje " << loop_name;
            *p_ss << "\n\tjne " << loop_name << "end";
          } else if (w.comparisons.cmp_s == ">=") {
            *p_ss << "\n\tjge " << loop_name;
            *p_ss << "\n\tjl " << loop_name << "end";
          } else if (w.comparisons.cmp_s == "<=") {
            *p_ss << "\n\tjle " << loop_name;
            *p_ss << "\n\tjg " << loop_name << "end";
          } else if (w.comparisons.cmp_s == ">") {
            *p_ss << "\n\tjg " << loop_name;
            *p_ss << "\n\tjle " << loop_name << "end";
          } else if (w.comparisons.cmp_s == "<") {
            *p_ss << "\n\tjl " << loop_name;
            *p_ss << "\n\tjle " << loop_name << "end";
          } else if (w.comparisons.cmp_s == "!=") {
            *p_ss << "\n\tjne " << loop_name;
            *p_ss << "\n\tje " << loop_name << "end";
          }
          *p_ss << "\n" << loop_name << "end:";
          /*gen->exitScope();*/
        }
        void operator()(NodeIfStmt ifs) {

          auto loop_id = gen->loop_count++;
          auto if_name = "if" + std::to_string(loop_id);
          // generate the condition-

          struct visitor {
            AsmGen *gen;
            std::stringstream *p_ss;
            bool lhs = true;
            void operator()(const std::shared_ptr<NodeFuncCall> &f) {
              // check for function definition
              bool _func_defined = false;
              for (auto &_func : gen->funcStack) {
                if (_func.identifier.value == f->identifier.value) {
                  _func_defined = true;
                }
              }
              if (_func_defined) {
                *p_ss << "\n\tcall " << f->identifier.value.value();
                if (!lhs)
                  *p_ss << "\n\tmov rcx, rax";
                else
                  *p_ss << "\n\tmov rax, rax";
              } else {
                printf("Error : Use of undeclared identifier \"%s\"\n",
                       f->identifier.value.value().c_str());
                exit(1);
              }
            }
            void operator()(NodeInt i) {
              if (!lhs)
                *p_ss << "\n\tmov cx, " + i.value.value.value();
              else
                *p_ss << "\n\tmov ax, " + i.value.value.value();
            }
            void operator()(Token t) {
              bool found_token = false;
              std::string var_name;
              for (int i = 0; i < gen->scope_stack.size(); i++) {
                var_name = gen->scope_stack.at(i) + "." + t.value.value();
                if (i > gen->scope_stack.size() - 1) {
                  printf("Use of Undeclared Identifier \"%s\"\n",
                         var_name.c_str());
                  // exit(1);
                } else {
                  for (auto v : gen->varScopes[gen->scope_stack.at(i)]) {
                    if (var_name == v.name.value.value()) {
                      found_token = true;
                      break;
                    }
                  }
                  if (found_token)
                    break;
                }
              }
              if (!found_token) {
                printf("Use of Undeclared Identifier \"%s\"\n",
                       var_name.c_str());
                // exit(1);
              }
              // i move the value at the address of the variable into eax
              // moving the label will move the memory address which is
              // different from the actual value
              if (!lhs)
                *p_ss << "\n\tmov cx, [" + var_name + "]";
              else
                *p_ss << "\n\tmov ax, [" + var_name + "]";
            }
          };

          std::visit(visitor{gen, p_ss, true}, ifs.condition->lhs);
          if (!ifs.condition->is_single_condition) {
            std::visit(visitor{gen, p_ss, false}, ifs.condition->rhs);
          } else {
            *p_ss << "\n\tmov cx, 0";
            ifs.condition->cmp_s = ">";
          }
          // compare

          std::string else_name = if_name + ".else";
          std::string _next_Block_ident = else_name;

          if (ifs.branches.size() != 0) {
            _next_Block_ident = "if" + std::to_string(loop_id + 1);
          }
          std::string _next_Block_header;
          if (ifs.branches.size() != 0)
            _next_Block_header = "elif." + _next_Block_ident;
          else
            _next_Block_header = _next_Block_ident;

          static std::string _parent;
          static bool _parent_checked = false;
          if (!ifs.is_elif && !_parent_checked) {
            _parent = if_name;
            _parent_checked = true;
          }

          *p_ss << "\n\tcmp ax, cx";
          if (ifs.condition->cmp_s == "==") {
            *p_ss << "\n\tje " + if_name;
            *p_ss << "\n\tjne " + _next_Block_header;
          } else if (ifs.condition->cmp_s == ">=") {
            *p_ss << "\n\tjge " + if_name;
            *p_ss << "\n\tjl " + _next_Block_header;
          } else if (ifs.condition->cmp_s == "<=") {
            *p_ss << "\n\tjle " + if_name;
            *p_ss << "\n\tjg " + _next_Block_header;
          } else if (ifs.condition->cmp_s == ">") {
            *p_ss << "\n\tjg " + if_name;
            *p_ss << "\n\tjle " + _next_Block_header;
          } else if (ifs.condition->cmp_s == "<") {
            *p_ss << "\n\tjl " + if_name;
            *p_ss << "\n\tjge " + _next_Block_header;
          } else if (ifs.condition->cmp_s == "!=") {
            *p_ss << "\n\tjne " + if_name;
            *p_ss << "\n\tje " + _next_Block_header;
          }

          *p_ss << "\n" + if_name + ":";
          // generate the body
          gen->changeScope(if_name);
          gen->generate(ifs.trueBody, *p_ss, *p_ss,
                        gen->varScopes[gen->scope_stack.back()]);
          *p_ss << "\n\tjmp " << _parent + "end";
          gen->exitScope();
          for (int i = 0; i < ifs.branches.size(); i++) {
            *p_ss << "\n" << _next_Block_header << ":";
            auto _stmt = ifs.branches.at(i);
            std::vector<NodeStmts> stmts;
            NodeStmts stmt;
            stmt.var = *_stmt;
            stmts.push_back(stmt);
            gen->generate(stmts, *p_ss, *p_ss,
                          gen->varScopes[gen->scope_stack.back()]);
            _next_Block_ident = "if" + std::to_string(loop_id);
            if (i + 1 == ifs.branches.size()) {

              *p_ss << "\n\tjmp " << if_name + "end";
            }
          }
          // else body
          gen->changeScope(if_name + ".else");

          *p_ss << "\n" + if_name + ".else:";
          gen->generate(ifs.falseBody, *p_ss, *p_ss,
                        gen->varScopes[gen->scope_stack.back()]);

          *p_ss << "\n\tjmp " + if_name + "end";

          *p_ss << "\n" + if_name + "end:";
          /*if (ifs.is_elif)*/
          /**p_ss << "\n\tjmp " << _parent + "end";*/

          gen->exitScope();
          _parent_checked = false;
        }
        void operator()(NodeReValStmt rv) {
          std::string var_name =
              gen->scope_stack.back() + "." + rv.identifier.value.value();
          bool _found = false;

          Var _var;
          for (auto &_item : gen->varScopes[gen->scope_stack.back()]) {
            if (_item.name.value.value() == var_name) {
              _var = _item;
              _found = true;
              break;
            }
          }

          if (!_found) {
            printf("Use of undeclared identifier \"%s\"\n",
                   rv.identifier.value.value().c_str());
            exit(1);
          }

          std::string _nv;
          struct _visitor {
            Token *ident;
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeReValStmt pm;
            std::string *var_name;

            void operator()(NodeString s) {

              /*  printf("Fatal : Cannot pass value of type "*/
              /*         "\"str\" to \"%s\"\n",*/
              /*         pm.identifier.value.value().c_str());*/
              /*  exit(1);*/
              /*}*/
              printf("Error : Cannot Concatnate string currently\n");
              exit(1);
            }

            void operator()(NodeInt i) {

              /*    printf("Fatal : Cannot pass value of type "*/
              /*           "\"int\" to \"%s\"\n",*/
              /*           pm.identifier.value.value().c_str());*/
              /*    exit(1);*/
              /*  }*/
              /*}*/
              /**p_ss << "\n\tmov rax, " << i.value.value.value();*/
              *var_name = i.value.value.value();
            }
            void
            operator()(const std::shared_ptr<std::vector<NodeBinaryExpr>> &b) {
              gen->gen_expr(b, p_ss);
              *p_ss << "\n\tpop rax";
              /**p_ss << "\n\tmov [" + var_name + "], rax";*/
              *var_name = "rax";
            }
            void operator()(const std::shared_ptr<NodeFuncCall> &s) {
              std::vector<NodeStmts> stmt;
              stmt.push_back({.var = *s});
              gen->generate(stmt, *p_ss, *p_ss,
                            gen->varScopes[gen->scope_stack.back()]);
              *p_ss << "\n\tmov rdi, rax";
              *var_name = "rax";
            }
            void operator()(NodeCmp &c) {}
            void operator()(Token t) {
              // reserving space for the variable
              // check if the value beign passed is valid
              auto _newValue = gen->scope_stack.back() + "." + t.value.value();
              bool found = false;
              Var _tester;
              for (auto &_var : gen->varScopes[gen->scope_stack.back()]) {
                if (_var.name.value.value() == _newValue) {
                  found = true;
                  _tester = _var;
                }
              }
              if (found) {
                // type checking

                /*  printf("Cannot pass value of \"%s\" to \"%s\" :: Invalid
                 * "*/
                /*         "Type Matching\n",*/
                /*         t.value.value().c_str(),*/
                /*         pm.identifier.value.value().c_str());*/
                /*  exit(1);*/
                /*}*/
                /**p_ss << "\n\tmov rax, ["*/
                /*      << gen->scope_stack.back() + "." + t.value.value() <<
                 * "]";*/
                /**p_ss << "\n\tmov [" << var_name << "], rax";*/
              } else {
                printf("Error : \"%s\" is not declared in this scope\n",
                       t.value.value().c_str());
                exit(1);
              }
              Var v;
              v.value = {.value = t.value.value()};
              /*v.type = pm.type;*/
              gen->varScopes[gen->scope_stack.back()].push_back(v);
            }
          };
          std::visit(_visitor{&rv.identifier, gen, p_ss, rv, &_nv},
                     rv.new_value->var);

          if (rv.opr == "+=") {
            *p_ss << "\n\tpush rax";
            *p_ss << "\n\tpush rcx";
            *p_ss << "\n\tmov rcx, [" << var_name << "]";
            *p_ss << "\n\tadd rcx, " << _nv;
            *p_ss << "\n\tmov [" + var_name << "], " << "rcx";
            *p_ss << "\n\tpop rcx";
            *p_ss << "\n\tpop rax";
          } else if (rv.opr == "-=") {
            *p_ss << "\n\tpush rax";
            *p_ss << "\n\tpush rcx";
            *p_ss << "\n\tmov rcx, [" << var_name << "]";
            *p_ss << "\n\tsub rcx, " << _nv;
            *p_ss << "\n\tmov [" + var_name << "], " << "rcx";
            *p_ss << "\n\tpop rcx";
            *p_ss << "\n\tpop rax";
          } else if (rv.opr == "*=") {
            *p_ss << "\n\tpush rax";
            *p_ss << "\n\tpush rcx";
            *p_ss << "\n\tmov rcx, [" << var_name << "]";
            *p_ss << "\n\timul rcx, " << _nv;
            *p_ss << "\n\tmov [" + var_name << "], " << "rcx";
            *p_ss << "\n\tpop rcx";
            *p_ss << "\n\tpop rax";
          }
        }
        void operator()(NodeIncludeStmt i) {
          i.header.erase(i.header.size() - 1, 1);
          i.header.append(".asm\"");
          gen->HEADER << "\n\t%include " << i.header;
        }
        void operator()(NodeBundle b) { gen->bundles.push_back(b.alias); }
        void operator()(NodeScopeRes r) {
          Logger::Trace("Checking Scope");
          bool found_alias = false;
          // replace with Semantics
          for (auto &alias : gen->bundles) {
            if (alias == r.scope_alias) {
              found_alias = true;
              break;
            }
          }

          if (!found_alias) {
            for (auto &ex_alias : extrn_namespaces) {
              if (ex_alias.first == r.scope_alias) {
                found_alias = true;
                break;
              }
            }
            if (!found_alias) {
              Semantics::Throw_Error("No Such Alias");
            }
          }
          gen->cur_nameSpace = r.scope_alias;
          struct visitor {
            AsmGen *gen;
            NodeScopeRes r;
            bool is_external = false;
            bool is_func = false;
            std::stringstream *p_ss;
            void operator()(NodeFuncCall fc) {
              if (!Semantics::find_symbol(extern_varScopes[gen->cur_nameSpace],
                                          fc.identifier, Semantics::func)) {
                Semantics::Throw_Error("symbol '%s' could not be resolved",
                                       fc.identifier.value.value().c_str());
              }
              fc.is_extrn = true;
              NodeProg funcC;
              funcC.stmt.push_back({.var = fc});
              gen->generate(funcC.stmt, *p_ss, *p_ss,
                            gen->varScopes[gen->scope_stack.back()]);
            }
            void operator()(std::string s) { printf("sttttring\n"); }
          };
          std::visit(visitor{gen, r, true, true, p_ss}, r.scope_member);
        }
      };

      std::visit(stmtVisitor{this, &curScope, &n_s, &p_s}, stmt.var);
    }
    Logger::Trace("End of Generating\n");
  }
  inline ~AsmGen() { printf("Done Generating\n"); }
};
