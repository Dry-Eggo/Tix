#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
enum BaseType {
  Void,
  Bool,
  I8,
  I16,
  I32,
  I64,
  U8,
  U16,
  U32,
  U64,
  F32,
  F64,
  Ptr,
  Str,
  Func,
  Custom
};

struct Type {
  BaseType base;
  bool is_mut = false;
  bool is_ptr = false;
  bool is_const = false;
  std::optional<std::string> name;

  struct FnSig {
    std::vector<Type> params;
    Type *return_type;
  };

  std::optional<FnSig> fn_sig;

  static Type make_ptr(Type inner) {
    auto t = inner;
    t.is_ptr = true;
    return t;
  }
  bool is_integer() {
    if (is_ptr) {
      return false;
    }
    switch (base) {
    case I32:
      return true;
    case I8:
      return true;
    case I16:
      return true;
    case I64:
      return true;
    case U8:
      return true;
    case U16:
      return true;
    case U32:
      return true;
    case U64:
      return true;
    default:
      return false;
    }
  }
};

enum class NodeKind {
  Literal,
  Variable,
  BinaryOp,
  UnaryOp,
  Call,
  Assignment,
  Return,
  VarDecl,
  Block,
  FnDecl,
  IF,
  Match,
  StructDecl,
  EnumDecl,
  FieldAccess,
  Index,
  Cast,
  While,
  Break,
  Continue,
  ExprStmt,
  Expr
};

struct Node {
  virtual ~Node() = default;
  virtual NodeKind kind() const = 0;
};

struct Expr : Node {
  std::optional<Type> resolved_type;
  NodeKind kind() const override { return NodeKind::Expr; }
};

struct LiteralExpr : Expr {
  std::variant<int64_t, double, std::string, bool> value;
  NodeKind kind() const override { return NodeKind::Literal; }
};

struct VarExpr : Expr {
  std::string name;
  VarExpr(std::string n) : name(std::move(n)) {}
  NodeKind kind() const override { return NodeKind::Variable; }
};

struct BinaryExpr : Expr {
  std::string op;
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
  NodeKind kind() const override { return NodeKind::BinaryOp; }
};

struct FunctionCallExpr : Expr {
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;
};

struct UnaryExpr : Expr {
  std::string op;
  std::unique_ptr<Expr> operand;
  NodeKind kind() const override { return NodeKind::UnaryOp; }
};

struct AssignmentExpr : Expr {
  std::string op;
  std::unique_ptr<Expr> target;
  std::unique_ptr<Expr> value;
  NodeKind kind() const override { return NodeKind::Assignment; }
};

struct Stmt : Node {};

struct VarDecl : Stmt {
  std::string name;
  Type type;
  std::unique_ptr<Expr> value;
  bool is_mut = false;
  bool is_const = false;

  NodeKind kind() const override { return NodeKind::VarDecl; }
};

struct ExprStmt : Stmt {
  std::unique_ptr<Expr> expr;
  NodeKind kind() const override { return NodeKind::ExprStmt; }
};

struct BlockStmt : Stmt {
  std::vector<std::unique_ptr<Stmt>> statements;
  NodeKind kind() const override { return NodeKind::Block; }
};

struct FnDecl : Node {
  std::string name;
  std::vector<std::pair<std::string, Type>> params;
  std::optional<Type> return_type;
  std::unique_ptr<BlockStmt> body;
  bool is_extern = false;

  std::optional<std::string> abi = std::nullopt;
  NodeKind kind() const override { return NodeKind::FnDecl; }
};

struct AST {
  std::vector<std::unique_ptr<FnDecl>> functions;
  std::vector<std::unique_ptr<VarDecl>> globals;
};
