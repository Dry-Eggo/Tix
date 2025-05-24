#pragma once

#include "node.hpp"
#include <string>
#include <utility>
#include <vector>

typedef std::vector<std::pair<std::string, Type>> params;
#include <string>
struct Function {
  std::string name;
  params args;
  bool is_extern = false;
  Type return_type = {BaseType::Void};
};
