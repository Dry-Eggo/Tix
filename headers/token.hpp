#pragma once

#include <cstdarg>
#include <cstddef>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

enum errType {
  ex_Delimiter,
  ex_Expression,
  ex_Integer,
  ex_Type,
  ex_Operator,
  ex_Oparen,
  ex_Obrace,
  ex_Cparen,
  ex_Cbrace,
  ms_Type,
  ms_TypeDef,
  un_Type,
  ex_Func,
  ms_Param,
  ms_Scope,
};

struct Error {
  errType type;
  int line;
  size_t col;
};

enum TokenType {
  SEMI,
  INT_LIT,
  OPAREN,
  CPAREN,
  IDENT,
  EXIT,
  MK,
  ASSIGN,
  STRING_LIT,
  TYPE_DEC,
  TYPE,
  SCOPE_RES,
  AS,
  FOR,
  ADD_EQU, // +=
  LTH,     // <
  N_EQU,   // !=
  EQU,     // ==
  SUB_EQU, // -=
  MUL_EQU, // *=
  DIV_EQU, // /=
  CBRACE,
  OBRACE,
  GTH,     // >
  LTH_EQU, // <=
  GTH_EQU, // >=
  FUNC,
  CALL,
  eof,
  ADD,
  SUB,
  DIV,
  MUL,
  EXTERN,
  WHILE,
  RET,
  IF,
  ELSE,
  R_PTR,
  ELIF,
  TRUE,
  FALSE,
  INCLUDE,
};

enum DataType {
  STR,
  INT,
  BOOL, // int 0 and 1 ::: translates bools into a possible 0 or 1. 0 beign
        // false, 1 beign true
  R_PTR_T,
};

struct Token {
  std::optional<std::string> value;
  TokenType type;
  int line, col;
  bool is_ptr = false;
};

struct NodeInt {
  Token value;
};

struct NodeString {
  Token value;
};

struct NodeBinaryExpr;
struct NodeExpr;

struct NodeReStmt {
  Token identifier;
  std::shared_ptr<NodeExpr> new_value;
};

struct NodeReValStmt {
  Token identifier;
  std::string opr;
  std::shared_ptr<NodeExpr> new_value;
};

struct NodeExitStmt {
  std::shared_ptr<NodeExpr> expr;
};

struct NodeMkStmt {
  Token identifier;
  DataType type;
  std::shared_ptr<NodeExpr> value;
  bool is_initialized = true;
};

struct NodeStmts;
struct NodeFuncCall;

struct NodeCmp {
  std::variant<std::shared_ptr<NodeFuncCall>, NodeInt, Token> lhs;
  std::string cmp_s;
  std::variant<std::shared_ptr<NodeFuncCall>, NodeInt, Token> rhs;
  bool is_single_condition = false;
};

struct NodeForStmt {
  Token identifier;
  Token startValue;
  std::shared_ptr<NodeCmp> condition;
  NodeCmp increment;

  std::vector<NodeStmts> body;
};

struct NodeWhileStmt {
  NodeCmp comparisons;
  std::vector<NodeStmts> body;
};

struct NodeIfStmt {
  Token identifier;
  std::shared_ptr<NodeCmp> condition;
  std::vector<NodeStmts> trueBody;
  std::vector<std::shared_ptr<NodeIfStmt>> branches;
  std::vector<NodeStmts> falseBody;
  bool has_else = false;
  bool is_elif = false;
};

struct Var {
  Token name;
  size_t stackOffset = 1;
  Token value;
  bool is_prt = false;
  bool is_function = false;
  DataType type;
  bool is_initialized = false;
};

struct NodeParam {
  Token identifier;
  DataType type;
  std::variant<Token, NodeInt, NodeString> value;
  size_t stackOffset = 1;
  std::string mangled_name;
  bool isptr = false;
};

struct NodeRet {
  std::shared_ptr<NodeExpr> value;
};

struct NodeFuncStmt {
  std::vector<NodeParam> params;
  std::vector<NodeStmts> body;
  DataType ret_type;
  size_t param_count = 0;
  Token identifier;
  NodeRet ret_value;
  bool has_ret = false;
  std::string mangled_name;
};

struct NodeFuncCall {
  Token identifier;
  std::vector<NodeParam> params;
  size_t param_count = 0;
  bool is_extrn = false;
  std::string ns = "glob";
};

struct NodeExtrnStmt {
  Token identifier;
  std::vector<NodeParam> param;
  size_t param_count = 0;
};

struct NodeCallStmt {
  Token std_lib_value;
  std::vector<NodeParam> params;
};

struct NodeExpr {
  std::variant<std::shared_ptr<std::vector<NodeBinaryExpr>>, NodeInt,
               NodeString, std::shared_ptr<NodeFuncCall>, NodeCmp, Token>
      var;
};

struct NodeIncludeStmt {
  std::string header;
  Token alias;
};

struct NodeBundle {
  std::string alias;
  std::vector<NodeStmts> contents;
  bool is_from_header = false;
};

struct NodeScopeRes {
  std::string scope_alias;
  std::variant<std::string, NodeFuncCall> scope_member;
};
struct NodeBinaryExpr {
  std::variant<NodeInt, NodeFuncCall, Token> lhs;
  std::string op;
  std::variant<NodeInt, NodeFuncCall, Token> rhs;
  bool has_rhs = true;
};
struct NodeStmts {
  std::variant<NodeExitStmt, NodeMkStmt, NodeReStmt, NodeReValStmt, NodeForStmt,
               NodeFuncStmt, NodeFuncCall, NodeCallStmt, NodeExtrnStmt,
               NodeWhileStmt, NodeIfStmt, NodeIncludeStmt, NodeBundle,
               NodeScopeRes>
      var;
};

struct NodeProg {
  std::vector<NodeStmts> stmt;
};

struct GenPrompt {
  std::vector<NodeStmts> stmts;
  std::stringstream ss;
  std::vector<Var> scope;
};
