#include "./src/backend/parser.hpp"
#include "./src/frontend/lexer.hpp"
#include "./src/tix.hpp"
#include "src/backend/generator.hpp"
#include <cstdio>
int main(int c, char **argv) {
  auto tix_process = Tix::Cli::Parse_Argv(c, argv);
  TixLexer *lexer = new TixLexer(tix_process);
  printf("Done Lexing\n");
  TixParser *parser = new TixParser(tix_process);
  printf("Done Parsing\n");
  Generator *gen = new Generator(tix_process);
  printf("Done Constructing\n");
  return 0;
}
