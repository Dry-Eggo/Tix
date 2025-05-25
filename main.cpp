#include "./src/backend/parser.hpp"
#include "./src/frontend/lexer.hpp"
#include "./src/frontend/stageprinter.hpp"
#include "./src/tix.hpp"
#include "src/backend/generator.hpp"
#include <cstdio>
#include <unistd.h>
int main(int c, char **argv) {
  StagedPrinter printer;
  printer.add_stage("Initializing");
  auto tix_process = Tix::Cli::Parse_Argv(c, argv, &printer);
  sleep(1);
  printer.mark_stage_done("Initializing");
  tix_process->printer = &printer;
  printer.add_stage("Lexing");
  TixLexer *lexer = new TixLexer(tix_process);
  sleep(1);
  printer.mark_stage_done("Lexing");
  printer.add_stage("Parsing");
  TixParser *parser = new TixParser(tix_process);
  sleep(1);
  printer.mark_stage_done("Parsing");
  printer.add_stage("LLVM Gen");
  Generator *gen = new Generator(tix_process);
  sleep(1);
  printer.mark_stage_done("LLVM Gen");
  printer.finalize();
  return 0;
}
