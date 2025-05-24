#pragma once

#include "variable.hpp"
#include <cstdio>
#include <iostream>
#include <optional>
#include <vector>
struct Context {
  std::vector<Variable> variables;
  Context *parent = nullptr;
  Context(Context *p) : parent(p) { variables = std::vector<Variable>(); }
  std::optional<Variable> search(std::string __query) {
    for (auto v : variables) {
      if (v.name == __query) {
        return v;
      }
    }
    if (parent)
      return parent->search(__query);
    return {};
  }

  void add(Variable __V) {
    variables.push_back(__V);
  }
};
