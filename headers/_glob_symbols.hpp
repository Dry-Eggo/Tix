#pragma once
#include "token.hpp"
#include <filesystem>
#include <map>
#include <vector>

namespace fs = std::filesystem;

std::map<std::string, std::vector<NodeFuncStmt>> extern_funcStack = {};
std::vector<std::string> search_paths;
std::map<std::string, std::vector<Var>> extern_varScopes = {};
std::map<std::string, std::vector<Var>> extrn_namespaces = {};
std::string _Tix_current_file;
