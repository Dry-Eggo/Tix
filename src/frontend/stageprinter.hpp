#pragma once

#include <asm-generic/ioctls.h>
#include <cstddef>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <sys/ioctl.h>
#include <type_traits>
#include <vector>
#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define CLG "\033[32m"
#define CLY "\033[33m"
#define CLReset "\033[0m"

struct SubStage {
  std::string name;
  bool done = false;

  std::string display() const {
    std::string prefix = done ? CLG "X" CLReset : CLY "*" CLReset;
    return " [" + prefix + "] " + name;
  }
};
struct Stage {
  std::string name;
  bool done = false;
  std::vector<SubStage> sub;

  std::string display() const {
    std::string prefix = done ? CLG "X" CLReset : CLY "*" CLReset;
    return " [" + prefix + "] " + name;
  }
};

struct StagedPrinter {
  std::vector<Stage> stages;
  std::string header = " [Tixc Compiling]";
  bool finalized = false;
  int lines_rendered = 0;

  void print_line(std::string content) { std::cout << content << std::endl; }

  void add_stage(std::string label) {
    if (finalized)
      return;
    stages.push_back(Stage{label, false});
    redraw();
  }

  void add_substage(std::string label, std::string sub) {
    for (auto &stage : stages) {
      if (stage.name == label) {
        stage.sub.push_back(SubStage{sub, false});
        break;
      }
    }
    redraw();
  }

  void mark_sub_stage_done(std::string label, std::string sub) {
    for (auto &stage : stages) {
      if (stage.name == label) {
        for (auto &sub_stage : stage.sub) {
          if (sub_stage.name == sub) {
            sub_stage.done = true;
            break;
          }
        }
      }
    }
    redraw();
  }
  void mark_stage_done(std::string label) {
    for (auto &stage : stages) {
      if (stage.name == label) {
        stage.done = true;
        break;
      }
    }
    redraw();
  }

  void move_cursor_up(int line) {
    if (line > 0)
      std::cout << "\033[" << line << "F";
  }
  void redraw() {
    /*clear_screen();*/
    move_cursor_up(lines_rendered);
    lines_rendered = 0;

    std::cout << header << "\n";
    lines_rendered++;

    for (int i = 0; i < stages.size(); ++i) {
      auto &stage = stages[i];
      std::string prefix;
      if (i == stages.size() - 1 && !stage.done) {
        prefix = " ├─────";
      } else if (i == stages.size() - 1) {
        prefix = " └─────";
      } else {
        prefix = " ├─────";
      }

      std::cout << prefix << stage.display() << std::endl;
      ++lines_rendered;
      for (int i = 0; i < stage.sub.size(); ++i) {
        auto &sub_stage = stage.sub[i];
        std::string prefix;
        if (i == stage.sub.size() - 1 && !sub_stage.done) {
          prefix = " │      ├─────";
        } else if (i == stage.sub.size() - 1) {
          prefix = " │       └─────";
        } else {
          prefix = " │       ├─────";
        }

        std::cout << prefix << sub_stage.display() << std::endl;
        ++lines_rendered;
      }
    }
    std::cout.flush();
  }
  void clear_screen() { std::cout << "\033[2J\033[H"; }
  void finalize() {
    add_stage("Compilation Complete");
    mark_stage_done("Compilation Complete");
  }
};
