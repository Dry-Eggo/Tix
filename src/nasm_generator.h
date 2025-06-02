#pragma once

#include "build/build_opt.h"
#include "context.h"
#include "lists/lists.h"
#include "node.h"
#include "symbol.h"
typedef struct NASM64_Function_Meta {
  const char *name;
  list_Param *parameters;
  Type return_type;
  bool has_return_value;
  const char *mangled_name;
} NASM64_Function_Meta;

TIX_DYN_LIST(NASM64_Function_Meta, NASM64_Function_Meta)
typedef struct {
  Program program;
  int rsp_loc;
  int current_offset;
  bool track_rsp_allocations;
  int total_rsp_size;
  NASM64_Context *global_context;
  NASM64_Context *current_context;
  list_NASM64_Function_Meta *functions;
} NASM64_generator;

void NASM64_init(NASM64_generator **, Program, char **, BuildOptions*);
void NASM64_generate(NASM64_generator *);
void NASM64_deinit(NASM64_generator *);
