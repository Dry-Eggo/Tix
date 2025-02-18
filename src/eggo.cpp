#include "_glob_symbols.hpp"
#include "eggoLog.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
int main(int argc, char **argv) {
  search_paths.push_back(fs::current_path());
  if (argc < 3) {
    Logger::Error({.type = errType::ex_Expression, .line = 0});
    exit(1);
  }
  std::string _out;
  std::string _file;
  for (int i = 0; i < argc; i++) {
    auto token = argv[i];
    if (token[0] == '-') {
      if (strcmp(token, "-s") == 0) {
        _file = argv[i + 1];
        fs::path _path_to_main = fs::current_path();
        _path_to_main = _path_to_main / _file;
        search_paths.push_back(_path_to_main.parent_path().string());
        Lexer lexer(_file.c_str());
      }
      if (strcmp(token, "-o") == 0) {
        _out = argv[i + 1];
      }
    }
  }

  std::stringstream ss;
  ss << "nasm -f elf64 -g -F dwarf " << _file << ".asm -o " << _file << ".o";

  system(ss.str().c_str());

  return 0;
}
