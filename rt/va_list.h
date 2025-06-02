
#pragma once
#include <stdarg.h>

typedef struct {
  int count;
  int index;
  void **args;
} TVAR_ARGS;

static void targ_init(TVAR_ARGS *arg) {
  arg->count = 0;
  arg->index = 0;
}

static void targ_push(TVAR_ARGS *args, void *arg) {
  args->args[args->count++] = arg;
}

#define poll(args, type) (*(type *)(args.args[args.index++]))
