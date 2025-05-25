#pragma once

#include "backend/node.hpp"
#include "backend/systems/Diagnostics.hpp"
#include "frontend/stageprinter.hpp"
#include "frontend/token.hpp"
#include <string>
#include <vector>
struct TixCompiler {
  std::string input_path;
  std::string output_path;
  std::string source_code;
  DiagnosticEngine diagnostics;
  std::vector<Token> tokens;
  AST *ast;
  StagedPrinter *printer;

  void error(std::string msg, Span loc, std::string note = "") {
    auto d = Diagnostic{DiagnosticLvl::Error, msg, note};
    d.addLabel(loc, note);
    diagnostics.report(d);
  }
  void showErrors() { diagnostics.flush(this->source_code); }
};

namespace Tix {
namespace Cli {
TixCompiler *Parse_Argv(int, char **, StagedPrinter *);
}
} // namespace Tix
