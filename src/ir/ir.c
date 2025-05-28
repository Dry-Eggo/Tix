#include "ir_gen.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static FILE *irfile;
void Env_init() {
  irfile = fopen("t.tir", "w");
  if (!irfile) {
    fprintf(stderr, "Fatal: Unable to open Ir file");
    exit(1);
  }
}
void IR_list_add(IR_list *l, TIR_Instruction *i) {
  if (l->count >= l->cap) {
    l->insts = realloc(l->insts, l->cap * 2);
    l->cap *= 2;
  }
  l->insts[l->count++] = i;
}
void ir_write(const char *fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  vfprintf(irfile, fmt, arg);
  va_end(arg);
  printf("Wrote %s to irfile\n", fmt);
}

void Env_close() { fclose(irfile); }
