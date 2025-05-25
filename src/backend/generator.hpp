#pragma once

#include "../tix.hpp"
#include "context.hpp"
#include "function.hpp"
#include "node.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <llvm/IR/Function.h>
#include <memory>
#include <optional>
#include <vector>

struct ExprResult {
  llvm::Value *value;
  Type type;
};

struct Generator {
  llvm::LLVMContext context;
  Context *current_context;
  std::vector<Function> functions;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module;
  llvm::Function *currentFunction = nullptr;
  TixCompiler *process;

  ExprResult gen_function_call(FunctionCallExpr *expr);
  void enter_scope();
  void exit_scope();
  void ensure_linear_match(std::string);
  std::optional<Function> get_function(Expr *);
  Generator(TixCompiler *process);
  llvm::Function *gen_function(FnDecl *);
  ExprResult gen_expression(Expr *);
  void gen_vardecl(VarDecl *);
  void gen_assignment(AssignmentExpr *);
  void gen_statement(Stmt *);
  bool type_match(Type, Type);
  llvm::Type *resolve_type(Type);
  void throw_error(std::string msg, int code);
};
