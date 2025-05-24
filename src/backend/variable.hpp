#pragma once

#include "node.hpp"
#include <llvm-19/llvm/IR/Instructions.h>
enum VarType {
  FN, VAR
};

struct Variable {
  VarType symtype;
  Type type;
  std::string name;
  llvm::AllocaInst *alloca;
  Variable(std::string name, Type ty, llvm::AllocaInst *alloca) {
    this->name = name;
    this->type = ty;
    this->alloca = alloca;
  }
};
