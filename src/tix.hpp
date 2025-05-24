#pragma once

#include "backend/node.hpp"
#include "frontend/token.hpp"
#include <string>
#include <vector>
struct TixCompiler {
  std::string input_path;
  std::string output_path;
  std::vector<Token> tokens;
  AST* ast;
};

namespace Tix {
namespace Cli {
TixCompiler *Parse_Argv(int, char **);
}
} // namespace Tix
