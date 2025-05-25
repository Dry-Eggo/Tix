#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

struct SourceLocation {
  std::string filename;
  int line;
  int colunm;

  SourceLocation(const std::string &filename = "", int ln = 0, int col = 0)
      : filename(filename), line(ln), colunm(col) {}
};

struct Span {
  SourceLocation start;
  SourceLocation end;
};

enum class DiagnosticLvl {
  Error,
  Warning,
  Trace,
};

struct Label {
  Span span;
  std::string message;
};

struct Diagnostic {
  DiagnosticLvl level;
  std::string message;
  std::vector<Label> labels;
  std::vector<std::string> suggestion;

  Diagnostic(DiagnosticLvl lvl, std::string msg, std::string hint)
      : level(lvl), message(msg) {}
  void addLabel(Span span, std::string msg) {
    labels.push_back({
        .span = span,
        .message = msg,
    });
  }
  void addNote(std::string note) { suggestion.push_back(note); }
};

struct DiagnosticEngine {
  std::vector<Diagnostic> diagnostics;
  std::string source_code;
  void report(Diagnostic diag) {
    printHeader(diag);
    for (const auto &label : diag.labels) {
      printLabel(diag, label);
    }
    for (const auto &note : diag.suggestion) {
      /*printNote(note);*/
    }
  }

  void printHeader(Diagnostic diag) {
    std::string levelstr;
    switch (diag.level) {
    case DiagnosticLvl::Error:
      levelstr = "Error";
      break;
    case DiagnosticLvl::Warning:
      levelstr = "Warning";
      break;
    case DiagnosticLvl::Trace:
      levelstr = "Note";
      break;
    }
    std::cout << levelstr << ": " << diag.message << "\n";
  }

  void printLabel(Diagnostic diag, Label label) {
    std::cout << " --> " << label.span.start.line << ":"
              << label.span.start.colunm << "\n";
    printSourceLine(source_code, label.span.start.line, label.span.end.colunm);

    if (!diag.suggestion.empty()) {
      std::cout << " := hint: ";
      for (auto s : diag.suggestion) {
        std::cout << s << "\n";
      }
    }
  }
  void flush(std::string_view source_code) {
    for (const auto &diag : diagnostics) {
    }
  }
  void printSourceLine(std::string_view src, int line, int col) {
    int curr_line = 1;
    std::string_view::size_type start = 0;
    while (start < src.size() && curr_line < line) {
      if (src[start] == '\n')
        curr_line++;
      start++;
    }
    std::string_view::size_type end = start;

    while (end < src.size() && src[end] != '\n')
      end++;

    std::string_view target_line = src.substr(start, end - start);
    std::cout << " |\n";
    std::cout << std::setw(2) << line << " | " << target_line << "\n";
    std::cout << " | " << std::string(col - 1, ' ') << "^\n";
  }
};
