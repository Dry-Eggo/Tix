#pragma once

#include "_glob_symbols.hpp"
#include "asmgen.hpp"
#include "eggoLog.hpp"
#include "token.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Parser {
public:
  AsmGen gen;
  inline Parser() {}
  inline void start(std::vector<Token> tokens, std::string _path,
                    bool _check_main = true) {
    this->tokens = std::move(tokens);
    while (peek().has_value()) {
      // if(peek().value().type == TokenType::EXIT)
      //{
      //   consume();
      // parse_exit_stmt();
      //}
      if (peek().value().type == TokenType::MK) {
        // consume();
        parse_mk_stmt(stmts.stmt);
      }
      if (peek().value().type == TokenType::IDENT) {
        // consume();
        parse_re_assign_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::EXTERN) {
        parse_extrn_stmt(stmts.stmt);
      }
      if (peek().value().type == TokenType::FOR) {
        parse_for_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::FUNC) {
        parse_func_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::CALL) {
        parse_call_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::IF) {
        parse_if_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::WHILE) {
        parse_while_stmt(stmts.stmt);
      }
      if (peek()->type == INCLUDE) {
        consume();
        NodeIncludeStmt inc_stmt;
        NodeStmts stmt;
        NodeBundle bndl;
        inc_stmt.header = consume().value.value();
        if (peek().has_value() && peek()->type == TokenType::AS)
          consume();
        if (peek().has_value() && peek()->type == TokenType::IDENT)
          inc_stmt.alias = consume();
        bndl.is_from_header = true;
        bndl.alias = inc_stmt.alias.value.value();
        stmt.var = inc_stmt;
        stmts.stmt.push_back(stmt);
        stmt.var = bndl;
        stmts.stmt.push_back(stmt);
      }
      if (peek()->type == TokenType::eof) {
        break;
      } else {
        // consume();
        //  e
      }
    }
    printf("size of program : %ld\n", stmts.stmt.size());
    gen.start(stmts, _path, _check_main);
  }
  inline ~Parser() {}

private:
  std::vector<Token> tokens;
  size_t m_index = 0;
  NodeProg stmts;

  inline std::optional<Token> peek(int i = 0) {
    if ((m_index + i) >= tokens.size()) {
      return {};
    }
    return tokens.at(m_index + i);
  }

  inline Token consume() { return tokens.at(m_index++); }

  inline void parse_call_stmt(std::vector<NodeStmts> &stmts) {
    NodeStmts stmt;
    NodeCallStmt call;
    consume(); // call
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      call.std_lib_value = consume();
      // stmt.var = call;
      // stmts.push_back(stmt);
      if (peek().has_value() && peek()->type == OPAREN) {
        consume();

        while (peek().has_value() && peek()->type != CPAREN) {
          NodeParam param;
          Token tkn;
          if (peek()->type == R_PTR) {
            consume();
            if (peek().has_value()) {
              tkn = consume();
              tkn.is_ptr = true;
            }
            param.value = tkn;
          } else if (peek()->type == IDENT) {
            tkn = consume();
            param.value = tkn;
          } else if (peek()->type == INT_LIT) {
            param.value = NodeInt{.value = consume()};
          } else if (peek()->type == STRING_LIT) {
            param.value = NodeString{.value = consume()};
          }
          call.params.push_back(param);
        }
        if (peek().value().type == TokenType::CPAREN) {
          consume();
          if (peek().value().type == TokenType::SEMI) {
            consume();
            stmt.var = call;
            stmts.push_back(stmt);
          }
        }
      }
    }
  }

  void parse_exit_stmt() {
    NodeStmts stmt;
    NodeExitStmt exitstmt;

    /*if (peek().value().type == TokenType::OPAREN) {*/

    /*             peek().value().type == TokenType::IDENT) {*/
    /*    exitstmt.expr.var = consume();*/
    /*    if (peek().has_value() && peek().value().type == TokenType::CPAREN)
     * {*/
    /*      consume();*/
    /*      if (peek().has_value() && peek().value().type == TokenType::SEMI)
     * {*/
    /*        stmt.var = exitstmt;*/
    /*        stmts.stmt.push_back(stmt);*/
    /*      } else {*/
    /*        Logger::Error({.type = errType::ex_Delimiter,*/
    /*                       .line = peek(-1).value().line,*/
    /*                       .col =
     * peek()->col+peek()->value.value().size()});*/
    /*        exit(1);*/
    /*      }*/
    /*    } else {*/
    /*      Logger::Error({.type = errType::ex_Cparen, .line =
     * peek(-1)->line});*/
    /*      exit(1);*/
    /*    }*/
    /*  }*/
    /*} else {*/
    /*  Logger::Error({.type = errType::ex_Oparen, .line = peek(-1)->line});*/
    /*  exit(1);*/
    /*}*/
  }

  inline void parse_mk_stmt(std::vector<NodeStmts> &stmts) {
    NodeStmts stmt;
    NodeMkStmt mkstmt;
    consume(); // discarding "MK" keyword
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      mkstmt.identifier = consume();
      if (peek().has_value() && peek()->type == TokenType::TYPE_DEC) {
        consume();
        if (peek().has_value() && peek()->type == TokenType::TYPE ||
            peek().has_value() && peek()->type == TokenType::IDENT) {
          // determining type;

          if (peek()->value == "str") {
            mkstmt.type = DataType::STR;
          } else if (peek()->value == "int") {
            mkstmt.type = DataType::INT;
          } else if (peek()->value == "bool") {
            mkstmt.type = DataType::BOOL;
          } else if (peek()->value == "buf") {
            mkstmt.type = DataType::R_PTR_T;
          }

          consume();
          if (peek().has_value() && peek()->type == TokenType::ASSIGN) {
            consume();
            if (peek().has_value()) {
              mkstmt.value =
                  std::make_shared<NodeExpr>(parse_expression(stmts));
            }
            if (peek().has_value() && peek()->type == SEMI) {
              consume();
            }
            stmt.var = mkstmt;
            stmts.push_back(stmt);
          } else if (peek().has_value() && peek()->type == TokenType::SEMI) {
            consume();
            mkstmt.is_initialized = false;
            stmt.var = mkstmt;
            stmts.push_back(stmt);
          }
        }
      }
    }
  }

  inline NodeFuncCall parse_re_assign_stmt(std::vector<NodeStmts> &stmts,
                                           bool push_back = true,
                                           bool check = true) {
    NodeStmts stmt;
    NodeReStmt restmt;

    if (peek(1).has_value() && peek(1).value().type == TokenType::ASSIGN) {
      restmt.identifier = consume();

      consume();
      if (peek().has_value() && peek().value().type == TokenType::INT_LIT ||
          peek().value().type == TokenType::IDENT ||
          peek().value().type == STRING_LIT) {
        if (peek()->type == INT_LIT)
          restmt.new_value =
              std::make_shared<NodeExpr>(parse_expression(stmts));
        else
          restmt.new_value =
              std::make_shared<NodeExpr>(parse_expression(stmts));

        if (peek().has_value() && peek().value().type == TokenType::SEMI) {
          consume();
          stmt.var = restmt;
          stmts.push_back(stmt);
        } else {
          Logger::Error(
              {.type = errType::ex_Delimiter, .line = peek(-1)->line});
          exit(1);
        }
      } else {
        Logger::Error({.type = errType::ex_Expression, .line = peek(-1)->line});
        exit(1);
      }
    } else if (peek(1).has_value() && peek(1)->type == TokenType::OPAREN) {
      NodeFuncCall fcall;
      fcall.ns = _Tix_current_file;
      int param_count = 0;
      fcall.identifier = consume();
      consume(); // opening parenthesis
      if (peek().has_value() && peek()->type != TokenType::CPAREN) {
        while (peek().has_value() && peek()->type != TokenType::CPAREN) {
          NodeParam param;
          Token tkn;
          if (peek()->type == R_PTR) {
            consume();
            if (peek().has_value()) {
              tkn = consume();
              tkn.is_ptr = true;
            }
            param.value = tkn;
          } else if (peek()->type == IDENT) {
            tkn = consume();
            param.value = tkn;
          } else if (peek()->type == INT_LIT) {
            param.value = NodeInt{.value = consume()};
          } else if (peek()->type == STRING_LIT) {
            param.value = NodeString{.value = consume()};
          }
          fcall.params.push_back(param);
          param_count++;
        }
      }
      if (peek().has_value() && peek()->type == TokenType::CPAREN &&
          peek(1).has_value()) {
        consume(); // )
        if (peek()->type == SEMI && check) {
          consume();
        } // ;
        fcall.param_count = param_count;
        stmt.var = fcall;
        if (push_back)
          stmts.push_back(stmt);
        return fcall;
      }
    } else if (peek(1).has_value() && peek(1)->type == SCOPE_RES) {
      NodeScopeRes scres;
      NodeStmts stmt;
      NodeFuncCall fc;
      if (peek()->type == TokenType::IDENT)
        scres.scope_alias = consume().value.value();
      consume();
      if (peek()->type == IDENT) {
        if (peek(1).has_value() && peek(1)->type == OPAREN) {
          fc = parse_re_assign_stmt(stmts, false, check);
          fc.is_extrn = true;
          fc.ns = scres.scope_alias;
          if (peek().has_value() && peek()->type == SEMI) {
            if (check)
              consume();
          }
          scres.scope_member = fc;
          stmt.var = scres;
          stmts.push_back(stmt);
          return fc;
        } else if (peek(1).has_value() && peek(1)->type == SEMI) {
          scres.scope_member = consume().value.value();
        } else {
          Logger::Trace("Ya messed up");
        }
      }
      if (peek().has_value() && peek()->type == SEMI) {
        if (check)
          consume();

      } else {
        Logger::Trace("Missing Semi Colon");
      }
      /*stmt.var = scres;*/
      /*stmts.push_back(stmt);*/
    } else if (peek(1).has_value()) {
      NodeStmts stmt;
      NodeReValStmt rev_stmt;

      rev_stmt.identifier = consume();
      switch (peek()->type) {
      case ADD_EQU:
        rev_stmt.opr = "+=";
        break;
      case SUB_EQU:
        rev_stmt.opr = "-=";
        break;
      case MUL_EQU:
        rev_stmt.opr = "*=";
        break;
      case DIV_EQU:
        rev_stmt.opr = "/=";
        break;
      default:
        break;
      }
      consume();
      if (peek().has_value())
        rev_stmt.new_value =
            std::make_shared<NodeExpr>(parse_expression(stmts));
      if (peek().has_value() && peek()->type == SEMI) {
        consume();
        stmt.var = rev_stmt;
        stmts.push_back(stmt);
      }
    }
    return {};
  }

  inline void parse_for_stmt(std::vector<NodeStmts> &stmts) {
    NodeForStmt for_stmt;
    NodeStmts stmt;

    consume(); // discard for keyword
    if (peek().has_value() && peek()->type == OPAREN)
      consume();
    // get the start value
    if (peek().has_value())
      for_stmt.identifier = consume(); //   counter

    if (peek().has_value() && peek()->type == TokenType::TYPE_DEC) //   :
      consume();
    else {
      Logger::Error({ms_Type, peek(-1)->line,
                     peek()->col + peek()->value.value().size()});
    }

    if (peek().has_value() && peek()->type == TokenType::TYPE) //    int
      consume();
    else {
      Logger::Error({ms_Type, peek(-1)->line,
                     peek()->col + peek()->value.value().size()});
    }

    if (peek().has_value() && peek()->type == TokenType::ASSIGN) //    =
      consume();
    else {
      Logger::Error({ex_Delimiter, peek(-1)->line,
                     peek()->col + peek()->value.value().size()});
    }

    if (peek().has_value()) //    10
      for_stmt.startValue = consume();

    if (peek().has_value() && peek()->type == TokenType::SEMI) //    ;
      consume();
    else {
      Logger::Error({ex_Delimiter, peek(-1)->line,
                     peek()->col + peek()->value.value().size()});
    }
    // get the condition

    if (peek().has_value())
      for_stmt.condition =
          std::make_shared<NodeCmp>(parse_condition(for_stmt.body));

    if (peek().has_value() && peek()->type == TokenType::SEMI) //    ;
      consume();
    else {
      Logger::Error({ex_Delimiter, peek(-1)->line, (size_t)peek(0)->col});
    }

    // get the increment value

    if (peek().has_value())
      for_stmt.increment = parse_condition(for_stmt.body);

    if (peek().has_value() && peek()->type == TokenType::CPAREN)
      consume();

    if (peek().has_value() && peek()->type == TokenType::OBRACE)
      consume();

    // parse the body
    if (peek().has_value() && peek()->type != TokenType::CBRACE) {
      while (peek().has_value() && peek()->type != CBRACE) {
        switch (peek().value().type) {
        case MK:
          parse_mk_stmt(for_stmt.body);
          break;
        case IDENT:
          parse_re_assign_stmt(for_stmt.body);
          break;
        case FOR:
          parse_for_stmt(for_stmt.body);
          break;
        case CALL:
          parse_call_stmt(for_stmt.body);
          break;
        case EXTERN:
          parse_extrn_stmt(for_stmt.body);
          printf(
              "Warning !: Declaration of external label in temporary scope\n");
          break;
        case IF:
          parse_if_stmt(for_stmt.body);
          break;
        default:
          printf("Invalid Statement\n");
          exit(1);
          break;
        }
      }
    }

    if (peek().has_value() && peek()->type == TokenType::CBRACE)
      consume();
    if (peek().has_value() && peek()->type == TokenType::SEMI) {
      consume();
      stmt.var = for_stmt;
      stmts.push_back(stmt);
    } else {
      Logger::Error({ex_Delimiter, peek(-1)->line,
                     peek()->col + peek()->value.value().size()});
    }
  }

  inline void parse_func_stmt(std::vector<NodeStmts> &stmts) {
    NodeStmts stmt;
    NodeFuncStmt func_stmt;
    consume(); // dicarding mk keyword
    int param_count = 0;
    if (peek()->type == TokenType::IDENT)
      func_stmt.identifier = consume();
    if (peek().has_value() && peek()->type == TokenType::OPAREN) {
      consume();
      if (peek()->type != CPAREN) {

        while (peek().has_value() && peek()->type != TokenType::CPAREN) {
          NodeParam param;
          if (peek()->type == TokenType::IDENT) {
            param.identifier = consume();
            if (peek().has_value() && peek()->type == TokenType::TYPE_DEC) {
              consume();
              if (peek().has_value() && peek()->type == TokenType::TYPE ||
                  peek().has_value() && peek()->type == TokenType::IDENT) {
                if (peek()->value == "str") {
                  param.type = DataType::STR;
                } else if (peek()->value == "int") {
                  param.type = DataType::INT;
                } else if (peek()->value == "bool") {
                  param.type = DataType::BOOL;
                } else if (peek()->value == "buf") {
                  param.type = DataType::R_PTR_T;
                }

                consume();
                func_stmt.params.push_back(param);
                param_count++;
              }
            }
          } else if (peek()->type == CPAREN)
            break;
        }
      }
      if (peek()->type == CPAREN)
        consume(); // closing parenthesis
                   //
      if (peek().has_value() && peek()->type == TYPE_DEC) {
        consume();
        if (peek().has_value() && peek()->type == TYPE) {
          func_stmt.has_ret = true;
          if (peek().has_value() && peek()->value == "str") {
            func_stmt.ret_type = DataType::STR;
          } else if (peek().has_value() && peek()->value == "int") {
            func_stmt.ret_type = DataType::INT;
          }

          consume();
        }
      }
      if (peek().has_value() && peek()->type == TokenType::OBRACE) {
        consume();
      }
      bool found_ret = false;
      if (peek()->type != CBRACE) {
        while (peek().has_value() && peek()->type != TokenType::CBRACE &&
               !found_ret) {
          if (peek()->type == TokenType::MK) {
            parse_mk_stmt(func_stmt.body);
          } else if (peek()->type == TokenType::IDENT) {
            parse_re_assign_stmt(func_stmt.body);
          } else if (peek()->type == TokenType::FOR) {
            parse_for_stmt(func_stmt.body);
          } else if (peek()->type == TokenType::EXIT) {
            parse_exit_stmt(); // ------    out of support ------- //
          } else if (peek()->type == TokenType::FUNC) {
            parse_func_stmt(func_stmt.body);
          } else if (peek()->type == TokenType::CALL) {
            parse_call_stmt(func_stmt.body);
          } else if (peek()->type == WHILE) {
            parse_while_stmt(func_stmt.body);
          } else if (peek()->type == IF) {
            parse_if_stmt(func_stmt.body);
          } else if (peek()->type == CBRACE) {
            break;
          } else if (peek()->type == RET) {
            consume();
            NodeRet ret;
            found_ret = true;
            switch (peek().value().type) {
            case INT_LIT:
              if (func_stmt.ret_type != DataType::INT) {
                Logger::Error(
                    {.type = ms_Type,
                     .line = peek(-1)->line,
                     .col = peek()->col + peek()->value.value().size()});
                exit(1);
              }
              break;
            case STRING_LIT:
              if (func_stmt.ret_type != DataType::STR) {
                Logger::Error(
                    {.type = ms_Type,
                     .line = peek(-1)->line,
                     .col = peek()->col + peek()->value.value().size()});
                exit(1);
              }
              break;
            case IDENT:
              break;

            default:
              break;
            }
            func_stmt.ret_value.value =
                std::make_shared<NodeExpr>(parse_expression(func_stmt.body));
            if (peek().has_value() && peek()->type == SEMI)
              consume();
          } // consume();
        }
      }

      if (peek()->type == CBRACE) {
        consume(); // closing curly
        if (peek()->type == SEMI)
          consume();
      }
      // consume();
      func_stmt.param_count = param_count;

      if (func_stmt.has_ret && !found_ret) {
        printf("This Function must return a value(%s)\n",

               func_stmt.identifier.value->c_str());
        exit(1);
      }
      stmt.var = func_stmt;
      stmts.push_back(stmt);
    }
  }
  inline NodeExpr parse_expression(std::vector<NodeStmts> &stmts) {
    NodeExpr expr;
    NodeStmts stmt;
    // Parsing general Experession

    if (peek().has_value() && peek()->type == TokenType::INT_LIT ||
        peek().has_value() && peek()->type == TokenType::IDENT) {
      if (peek(1).has_value() && peek(1)->type != SEMI) {
        if (peek(1)->type == SCOPE_RES || peek(1)->type == OPAREN) {
          expr.var = std::make_shared<NodeFuncCall>(
              parse_re_assign_stmt(stmts, false, false));
        } else
          expr.var =
              std::make_shared<std::vector<NodeBinaryExpr>>(parse_bexpr(stmts));
      } else {
        if (peek()->type == IDENT)
          expr.var = Token(consume());
        else if (peek()->type == INT_LIT)
          expr.var = NodeInt{consume()};
      }
    } else if (peek().has_value() && peek()->type == TokenType::STRING_LIT) {
      expr.var = NodeString{.value = consume()};
    }
    return expr;
  }
  std::vector<NodeBinaryExpr> parse_bexpr(std::vector<NodeStmts> &stmts) {
    std::vector<NodeBinaryExpr> b_expr;
    NodeExpr expr;

    bool lhs = true;
    while (peek().has_value() && peek()->type != SEMI) {
      NodeBinaryExpr ex;
      if (peek().has_value() && peek()->type == INT_LIT) {
        if (lhs) {
          ex.lhs = NodeInt{consume()};
          lhs = false;
        } else {
          ex.rhs = NodeInt{consume()};
          lhs = true;
        }
      } else if (peek().has_value() && peek()->type == IDENT) {
        if ((peek(1).has_value() && peek(1)->type == OPAREN) ||
            (peek(1).has_value() && peek(1)->type == SCOPE_RES)) {
          if (lhs) {
            ex.lhs = parse_re_assign_stmt(stmts, false, false);
            if (peek().has_value() && peek()->type == SEMI) {
              consume();
              break;
            }
            lhs = false;
          } else {
            ex.rhs = parse_re_assign_stmt(stmts, false, false);
            if (peek().has_value() && peek()->type == SEMI) {
              consume();
              break;
            }
            lhs = true;
          }
        }

        else {
          if (lhs) {
            ex.lhs = consume();
            lhs = false;
          } else {
            ex.rhs = consume();
            lhs = true;
          }
        }
      } else {
        switch (peek().value().type) {
        case TokenType::MUL:
          ex.op = "*";
          lhs = true;
          break;
        case TokenType::ADD:
          ex.op = "+";
          lhs = true;
          break;
        case TokenType::DIV:
          ex.op = "/";
          lhs = true;
          break;
        case TokenType::SUB:
          ex.op = "-";
          lhs = true;
          break;
        default:
          break;
        }

        consume();
      }
      if (lhs == true)
        ex.has_rhs = true;
      b_expr.push_back(std::move(ex));
    }
    return b_expr;
  }
  inline void parse_extrn_stmt(std::vector<NodeStmts> &stmts) {
    NodeExtrnStmt extrnStmt;
    NodeStmts stmt;
    consume(); // dmk
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      extrnStmt.identifier = consume();
      if (peek().has_value() && peek()->type == OPAREN) {
        consume();
        while (peek().has_value() && peek()->type != CPAREN) {
          NodeParam param;
          if (peek()->type == IDENT) {
            param.identifier = consume();
            if (peek().has_value() && peek()->type == TYPE_DEC) {
              consume();
              if (peek().has_value() && peek()->type == TYPE) {
                if (peek()->value == "int") {
                  param.type = INT;
                } else if (peek()->value == "str") {
                  param.type = STR;
                } else if (peek()->value == "bool") {
                  param.type = BOOL;
                }
                consume();
              }
              extrnStmt.param.push_back(param);
            }
          }
        }
        if (peek().has_value() && peek()->type == CPAREN) {
          consume();
        }
        if (peek().has_value() && peek()->type == SEMI) {
          consume();
          stmt.var = extrnStmt;
          stmts.push_back(stmt);
        }
      }
    }
  }
  inline void parse_while_stmt(std::vector<NodeStmts> &stmts) {
    NodeStmts stmt;
    NodeWhileStmt wh_stmt;

    if (peek().has_value() && peek()->type == TokenType::WHILE)
      consume();
    if (peek().has_value() && peek()->type == TokenType::OPAREN)
      consume();

    wh_stmt.comparisons = parse_condition(wh_stmt.body);
    if (peek().has_value() && peek()->type == TokenType::CPAREN)
      consume();

    if (peek().has_value() && peek()->type == TokenType::OBRACE)
      consume();
    wh_stmt.body = parse_body();
    stmt.var = wh_stmt;
    stmts.push_back(stmt);
  }
  void parse_if_stmt(std::vector<NodeStmts> &stmts) {
    NodeIfStmt if_stmt;
    NodeStmts stmt;

    if (peek().has_value() && peek()->type == TokenType::IF) {
      consume();
      if (peek().has_value() && peek()->type == OPAREN) {
        consume();
        if_stmt.condition =
            std::make_shared<NodeCmp>(parse_condition(if_stmt.trueBody));
        if (peek().has_value() && peek()->type == CPAREN) {
          consume();
          if (peek().has_value() && peek()->type == OBRACE) {
            if_stmt.trueBody = parse_body(false);
            if (peek().value().type == SEMI) {
              consume();
              stmt.var = if_stmt;
              stmts.push_back(stmt);
            } else {
              if (peek().value().type == ELIF) {
                while (peek()->type == ELIF) {
                  NodeIfStmt elif_stmt;
                  consume();
                  elif_stmt.is_elif = true;
                  if (peek().has_value() && peek()->type == TokenType::OPAREN)
                    consume();
                  elif_stmt.condition = std::make_shared<NodeCmp>(
                      parse_condition(elif_stmt.trueBody));
                  if (peek().has_value() && peek()->type == TokenType::CPAREN)
                    consume();
                  if (peek().has_value() && peek()->type == TokenType::OBRACE)
                    elif_stmt.trueBody = parse_body(false);
                  if (peek().value().type == TokenType::ELSE) {
                    consume();
                    if_stmt.has_else = true;
                    elif_stmt.falseBody = parse_body();

                    if_stmt.branches.push_back(
                        std::make_shared<NodeIfStmt>(elif_stmt));
                    if (peek().has_value() && peek()->type == SEMI) {
                      consume();
                      stmt.var = if_stmt;
                      stmts.push_back(stmt);
                    }
                  } else if (peek().has_value() && peek()->type == SEMI) {
                    if_stmt.branches.push_back(
                        std::make_shared<NodeIfStmt>(elif_stmt));
                    stmt.var = if_stmt;
                    stmts.push_back(stmt);
                    consume();
                    break;
                  }
                }
              } else if (peek().value().type == TokenType::ELSE) {
                consume();
                if_stmt.has_else = true;
                if_stmt.falseBody = parse_body();
                if (peek().has_value() && peek(-1)->type == SEMI) {
                  stmt.var = if_stmt;
                  stmts.push_back(stmt);
                }
              }
            }
          }
        }
      }
    }
  }
  NodeCmp parse_condition(std::vector<NodeStmts> &stmts) {
    NodeCmp cmp;
    if (peek().has_value() && peek()->type == INT_LIT) {
      cmp.lhs = NodeInt{.value = consume()};
    } else if (peek().has_value() && peek()->type == IDENT) {
      if (peek(1).value().type == OPAREN)
        cmp.lhs = std::make_shared<NodeFuncCall>(
            parse_re_assign_stmt(stmts, false, false));
      else
        cmp.lhs = consume();

    } else {
      printf("Fatal : Expected an Expression");
      exit(1);
    }

    if (peek().has_value() && peek()->type == EQU) {
      cmp.cmp_s = "==";
      consume();
    } else if (peek().has_value() && peek()->type == N_EQU) {
      cmp.cmp_s = "!=";
      consume();
    } else if (peek().has_value() && peek()->type == LTH) {
      cmp.cmp_s = "<";
      consume();
    } else if (peek().has_value() && peek()->type == ADD_EQU) {
      cmp.cmp_s = "+=";
      consume();
    } else if (peek().has_value() && peek()->type == LTH_EQU) {
      cmp.cmp_s = "<=";
      consume();
    } else if (peek().has_value() && peek()->type == GTH_EQU) {
      cmp.cmp_s = ">=";
      consume();
    } else if (peek().has_value() && peek()->type == SUB_EQU) {
      cmp.cmp_s = "-=";
      consume();
    } else if (peek().has_value() && peek()->type == GTH) {
      cmp.cmp_s = ">";
      consume();
    } else {
      cmp.is_single_condition = true;
    }

    if (peek().has_value() && peek()->type == INT_LIT) {
      cmp.rhs = NodeInt{.value = consume()};
    } else if (peek().has_value() && peek()->type == IDENT) {
      if (peek(1).value().type == OPAREN)
        cmp.rhs = std::make_shared<NodeFuncCall>(
            parse_re_assign_stmt(stmts, false, false));
      else
        cmp.rhs = consume();

    } else {
      cmp.is_single_condition = true;
    }

    return cmp;
  }
  std::vector<NodeStmts> parse_body(bool check = true) {
    std::vector<NodeStmts> body;
    if (peek().has_value() && peek()->type == TokenType::OBRACE)
      consume();

    while (peek().has_value() && peek()->type != TokenType::CBRACE) {
      switch (peek().value().type) {
      case TokenType::MK:
        parse_mk_stmt(body);
        break;
      case TokenType::IDENT:
        parse_re_assign_stmt(body);
        break;
      case TokenType::IF:
        parse_if_stmt(body);
        break;
      case TokenType::CALL:
        parse_call_stmt(body);
        break;
      case TokenType::FOR:
        parse_for_stmt(body);
        break;
      case TokenType::WHILE:
        parse_while_stmt(body);
        break;
      default:
        Logger::Trace("Not a supported statement");
        exit(1);
        break;
      }
    }
    if (peek().has_value() && peek()->type == TokenType::CBRACE)
      consume();
    if (peek().has_value() && peek()->type == TokenType::SEMI)
      if (check)
        consume();
    return body;
  }
};
