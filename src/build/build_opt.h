

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAG(arg, long, short)                                                 \
  (strcmp(arg, long) == 0 || strcmp(arg, short) == 0)
typedef struct {
  const char *help;
} ARGDef;
typedef enum {
  BUILD_TARGET = 0,
  OUTPUT = 1,
  VERSION = 2,
  NO_CMD,
} Help_target;
typedef struct {
  const char *inputfile;
  const char *outputfile;
  Help_target help_target;
  bool show_help;
  bool show_version;
  bool run;
  bool has_specialized_help_target;
} BuildOptions;

static const char *shift(int *c, char **v) { return v[(*c)++]; }

extern void print_usage(const char *);
static Help_target get_help_index(const char *target) {
  if (strcmp(target, "target") == 0) {
    return BUILD_TARGET;
  } else if (strcmp(target, "output") == 0) {
    return OUTPUT;
  } else if (strcmp(target, "version") == 0) {
    return VERSION;
  }
  return NO_CMD;
}

static void parse_arguments(BuildOptions *b, int c, char **v) {
  int i = 1;
  b->has_specialized_help_target = false;
  while (i < c) {
    const char *arg = shift(&i, v);
    if (FLAG(arg, "--help", "-h")) {
      if (i < c) {
        b->help_target = get_help_index(shift(&i, v));
        b->has_specialized_help_target = true;
        return;
      } else {
        b->has_specialized_help_target = false;
        b->show_help = true;
        return;
      }
    } else if (FLAG(arg, "--version", "-v")) {
      b->show_version = true;
      return;
    } else if (FLAG(arg, "--output", "-o")) {
      if (c <= 0) {
        print_usage(v[0]);
        fprintf(stderr, "\n--out expected an argument");
        exit(1);
      }
      b->outputfile = shift(&i, v);
    } else {
      if (arg[0] != '-' && b->inputfile) {
        fprintf(stderr, "tixc: Unknown Command '%s'\n", arg);
      } else if (arg[0] != '-') {
        b->inputfile = arg;
      }
    }
  }
  if (!b->inputfile) {
    print_usage(v[0]);
    printf("tixc: no input files where provided\n");
    exit(1);
  }
}
