#include "tix.hpp"
#include "frontend/stageprinter.hpp"
#include <cstdio>
#include <iostream>
#include <string>

namespace Tix {
namespace Cli {
TixCompiler *Parse_Argv(int c, char **v, StagedPrinter *printer) {
  TixCompiler *compiler = new TixCompiler();
  auto len = c;
  for (int i = 1; i < len; ++i) {
    std::string arg = v[i];
    if (arg == "compile") {
      std::string source;
      if (i + 1 >= len) {
        printer->add_substage("Initializing",
                              "\033[91mTix Error: No Source File Given\033[0m");
        printer->mark_sub_stage_done(
            "Initializing", "\033[91mTix Error: No Source File Given\033[0m");
        printer->finalize();
        exit(1);
      }
      source = v[++i];
      compiler->input_path = source;
    } else if (arg.substr(0, 1) == "-") {
      if (arg == "--output" || arg == "-o") {
        if (i + 1 >= len) {
          printer->add_substage(
              "Initializing", "\033[91mTix Error: No Output Path given\033[0m");
          printer->mark_sub_stage_done(
              "Initializing", "\033[91mTix Error: No Output Path given\033[0m");
          printer->finalize();
          exit(1);
        }
        compiler->output_path = v[++i];
      }
    }
  }
  return compiler;
}
} // namespace Cli
} // namespace Tix
