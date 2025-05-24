#include "generator.hpp"
#include "context.hpp"
#include "node.hpp"
#include "variable.hpp"
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

std::error_code EC;
llvm::raw_fd_ostream outfile("tix.ll", EC, llvm::sys::fs::OF_Text);

Generator::Generator(TixCompiler *process)
    : builder(context), current_context(new Context(nullptr)) {
  this->module = std::make_unique<llvm::Module>(process->input_path, context);
  for (auto &f : process->ast->functions) {
    this->gen_function(f.get());
  }
  this->module->print(outfile, nullptr);
}

llvm::Function *Generator::gen_function(FnDecl *fn) {
  llvm::FunctionType *fntype =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
  llvm::Function *func = llvm::Function::Create(
      fntype, llvm::Function::ExternalLinkage, fn->name, module.get());
  currentFunction = func;
  if (!fn->is_extern) {
    auto entry = llvm::BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(entry);
  }
  enter_scope();
  for (auto &stmt : fn->body->statements) {
    gen_statement(stmt.get());
  }
  exit_scope();
  Function function;
  function.args = fn->params;
  function.is_extern = fn->is_extern;
  function.name = fn->name;
  function.return_type = fn->return_type.value_or(Type{BaseType::Void});
  functions.push_back(function);
  builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
  return func;
}

llvm::Value *Generator::gen_expression(Expr *e) {
  if (auto *lit_int = dynamic_cast<LiteralExpr *>(e)) {
    if (std::holds_alternative<int64_t>(lit_int->value)) {
      auto i = std::get<int64_t>(lit_int->value);
      return llvm::ConstantInt::get(
          resolve_type(lit_int->resolved_type.value()), i);
    } else if (std::holds_alternative<std::string>(lit_int->value)) {
      auto s = std::get<std::string>(lit_int->value);
      return llvm::ConstantDataArray::getString(context, s, true);
    }
  } else if (auto *var = dynamic_cast<VarExpr *>(e)) {
    auto v = current_context->search(var->name);
    if (v.has_value()) {
      return llvm::Value(v->alloca->getType());
    }
  }

  return nullptr;
}

void Generator::gen_statement(Stmt *stmt) {
  if (auto *var_stmt = dynamic_cast<VarDecl *>(stmt)) {
    gen_vardecl(var_stmt);
  } else if (auto *expr = dynamic_cast<ExprStmt *>(stmt)) {
    if (auto *as_expr = dynamic_cast<AssignmentExpr *>(expr->expr.get())) {
      gen_assignment(as_expr);
    } else if (auto *fcall =
                   dynamic_cast<FunctionCallExpr *>(expr->expr.get())) {
      gen_function_call(fcall);
    }
  }
}

void Generator::gen_function_call(FunctionCallExpr *expr) {
  auto callee_expr = gen_expression(expr->callee.get());
  llvm::Function *func = llvm::dyn_cast<llvm::Function>(callee_expr);
  if (!func) {
    throw_error("Attempt To call Non function Value", 1);
  }
  std::vector<llvm::Value *> args;
  for (auto &a : expr->arguments) {
    auto arg_expr = gen_expression(a.get());
    args.push_back(arg_expr);
  }
  builder.CreateCall(func, args);
}

void Generator::gen_assignment(AssignmentExpr *expr) {
  if (auto *var = dynamic_cast<VarExpr *>(expr->target.get())) {
    auto symbol = this->current_context->search(var->name);
    if (!symbol.has_value()) {
      std::printf("Use of undeclared Identifier '%s'", var->name.c_str());
      exit(1);
    }

    llvm::BasicBlock &tmpEntry = currentFunction->getEntryBlock();
    llvm::IRBuilder<> tmpBuilder(&tmpEntry, tmpEntry.begin());
    llvm::AllocaInst *ptr = symbol->alloca;
    builder.CreateStore(gen_expression(expr->value.get()), ptr);
  }
}

void Generator::gen_vardecl(VarDecl *v) {
  auto subject = current_context->search(v->name);
  if (subject.has_value()) {
    throw_error(std::string("Redefinition of ") + v->name.c_str(), 1);
  }
  ensure_linear_match(v->name);
  auto ty = resolve_type(v->type);
  printf("Allocated Stack Space for Variable %s\n", v->name.c_str());
  if (ty == nullptr) {
    printf("Fatal: Type Resolution: NULL\n");
  }
  llvm::BasicBlock &tmpEntry = currentFunction->getEntryBlock();
  llvm::IRBuilder<> tmpBuilder(&tmpEntry, tmpEntry.begin());
  llvm::AllocaInst *ptr = tmpBuilder.CreateAlloca(ty, nullptr, v->name.c_str());
  builder.CreateStore(gen_expression(v->value.get()), ptr);
  printf("'%s' Allocation Successful\n", v->name.c_str());
  Variable var = Variable(v->name, v->type, ptr);
  this->current_context->add(var);
}

llvm::Type *Generator::resolve_type(Type t) {
  switch (t.base) {
  case BaseType::I32: {
    auto v = llvm::Type::getInt32Ty(context);
    printf("Resolved Type To 32bit Int\n");
    return v;
  } break;
  case BaseType::Str: {
    auto v = llvm::Type::getInt8Ty(context)->getPointerTo();
    std::printf("Resolved Type To String\n");
    return v;
  } break;
  case BaseType::I8: {
    auto v = llvm::Type::getInt8Ty(context);
    return v;
  }
  default:
    return nullptr;
  }
}

void Generator::enter_scope() {
  current_context = new Context(current_context);
}
void Generator::exit_scope() {
  Context *old_ctx = current_context;
  current_context = old_ctx->parent;
  delete old_ctx;
}

void Generator::throw_error(std::string msg, int code) {
  std::cerr << "Tix Error: " << msg << "\n\t";
  std::cerr << "Process Terminated with code: " << code << "\n";
  exit(code);
}
void Generator::ensure_linear_match(std::string __s) {
  for (auto f : functions) {
    if (f.name == __s) {
      throw_error(std::string("Redefinition of ") + __s, 1);
    }
  }
}
