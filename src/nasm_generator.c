
#include "nasm_generator.h"
#include "build/build_opt.h"
#include "context.h"
#include "internals.h"
#include "ir/nero.h"
#include "lexer.h"
#include "node.h"
#include "string_builder.h"
#include "symbol.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

StringBuilder *nasm_out;
static BuildOptions *buildOptions;
static StringBuilder *bss;
static StringBuilder *header;
static StringBuilder *data;
static StringBuilder *text;
static StringBuilder *rodata;
static char **Source;
static char *sysv_regs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char *x86_64_regs[6] = {"rdi", "rsi", "rdx", "r10", "r9", "r8"};
#define NASM(stream, ...) sbapp(text, stream, ##__VA_ARGS__);
#define STR_EQ(s1, s2) strcmp(s1, s2) == 0

typedef struct {
  Type ty;
  StringBuilder *opsize;
  StringBuilder *preamble, *result;
  bool is_initialized_expr;
  char *object_name; // strictly to pass ident awareness for function call
  char *reg_name;
} NASM64_ExprResult;
NASM64_ExprResult *new_expr_result() {
  NASM64_ExprResult *res = NEW(NASM64_ExprResult);
  SB_init(&res->result);
  SB_init(&res->preamble);
  SB_init(&res->opsize);
  res->is_initialized_expr = false;
  res->object_name = (char *)malloc(64);
  res->ty.is_signed = false;
  res->reg_name = NULL;
  return res;
}
typedef enum MovOpcode { MOVB, MOVD, MOVW, MOVSX, MOVZX, MOVQ } MovOpcode;
const char *MovOpcode_tostr(MovOpcode op) {
  switch (op) {
  case MOVB:
    return "byte";
  case MOVD:
    return "dword";
  case MOVQ:
    return "qword";
  case MOVSX:
    return "movsx";
  case MOVZX:
    return "movzx";
  case MOVW:
    return "word";
  }
}
void NASM64_Function_Meta_init(NASM64_Function_Meta **meta, struct FnStmt *fn) {
  *meta = NEW(NASM64_Function_Meta);
  (*meta)->name = fn->name;
  (*meta)->parameters = fn->param;
  (*meta)->return_type = fn->return_type;
  (*meta)->mangled_name = NULL;
}
bool NASM64_is_free_symbol_identifier(NASM64_generator *gen,
                                      const char *ident) {
  if (!NASM64_Context_rsearch(gen->current_context, ident)) {
    int max = gen->functions->count;
    for (int c = 0; c < max; c++) {
      if (strcmp(list_NASM64_Function_Meta_get(gen->functions, c)->name,
                 ident) == 0) {
        return false;
      }
    }
    return true;
  }
  return false;
}
MovOpcode MovOpcode_get_movopcode(Type type, bool is_signed, Type dst) {
  // if (is_signed)
  //   return MOVSX;
  if (type.size_in_bytes == dst.size_in_bytes) {
    if (dst.is_signed)
      return MOVSX;
    switch (type.size_in_bytes) {
    case 1:
      return MOVB;
    case 2:
      return MOVW;
    case 4:
      return MOVD;
    case 8:
      return MOVQ;
    default:
      break;
    }
  }
  if (type.size_in_bytes < dst.size_in_bytes) {
    if (is_signed)
      return MOVSX;
    return MOVZX;
  }
  return MOVQ;
}
int NASM64_align(int offset, int size, int align) {
  offset -= size;
  int rem = offset % align;
  if (rem != 0) {
    offset -= (align - rem);
  }
  return offset;
}
int NASM64_get_next_offset(NASM64_generator *gen) {
  return gen->current_offset;
}
// Escapes raw strings from source code i.e turning '\n' to actual newlines
const char *NASM64_string_escape(const char *str) {
  // TODO: add support for all escape sequences
  char buf[1024] = {0};
  int i = 0;
  int b = 1;
  buf[0] = '\'';
  while (str[i]) {
    if (str[i] == '\\' && str[i + 1] == 'n') {
      strcat(buf, "', 10");
      i += 2;
      b += 5;
      if (str[i]) {
        strcat(buf, ", '");
        b += 3;
      }
    }
    buf[b] = str[i];
    ++i;
    ++b;
  }
  buf[b] = '\'';
  return strdup(buf);
}
const char *NASM64_get_registerv(int i) { return sysv_regs[i]; }
const char *NASM64_reg_proper(const char *reg, Type ty) {
  if (ty.is_signed)
    return reg;
  if (STR_EQ(reg, "rdi")) {
    switch (ty.size_in_bytes) {
    case 1:
      return "dil";
    case 2:
      return "di";
    case 4:
      return "edi";
    default:
      break;
    }
  } else if (STR_EQ(reg, "rsi")) {
    switch (ty.size_in_bytes) {
    case 1:
      return "sil";
    case 2:
      return "si";
    case 4:
      return "esi";
    default:
      break;
    }
  }
  // TODO: add all the x86_64 registers
  return reg;
}
// Aligns the next offset to 4 bytes
// status: Unimplemented
//           increments the current offset for now
void NASM64_set_next_offset_to_next_multiple_of_4(NASM64_generator *gen) {
  gen->current_offset++;
}

void NASM64_init(NASM64_generator **n, list_NERO_Inst* p, char **source,
                 BuildOptions *b) {
  *n = (NASM64_generator *)malloc(sizeof(NASM64_generator));
  (*n)->program = p;
  (*n)->rsp_loc = (*n)->current_offset = 0;
  (*n)->current_offset = 0;
  (*n)->rsp_loc = 0;
  (*n)->total_rsp_size = 0;
  (*n)->track_rsp_allocations = false;
  NASM64_Context_init(&(*n)->global_context, NULL);
  (*n)->current_context = (*n)->global_context;
  list_NASM64_Function_Meta_init(&(*n)->functions);
  if (!b->outputfile) {
    b->outputfile = "t.s";
  }
  buildOptions = b;
  SB_init(&data);
  SB_init(&text);
  SB_init(&header);
  SB_init(&rodata);
  SB_init(&bss);
  Source = source;
}
void NASM64_enter_context(NASM64_generator *gen, NASM64_Context *ctx) {
  gen->current_context = ctx;
}
void NASM64_exit_context(NASM64_generator *gen, NASM64_Context *ctx) {
  if (ctx->parent) {
    gen->current_context = ctx->parent;
    free(ctx);
    return;
  }
  gen->current_context = gen->global_context;
}
int NASM64_get_type_size(Type ty) {
  if (ty.is_ptr)
    return 8;
  switch (ty.base) {
  case TTYPE_I32:
    return 4;
  case TTYPE_U8:
    return 1;
  default:
    TIX_LOG(stderr, ERROR, "Invalid Type");
    exit(1);
  }
}

NASM64_Function_Meta *NASM64_has_function(NASM64_generator *gen,
                                          const char *name) {
  int max = gen->functions->count;
  for (int i = 0; i < max; ++i) {
    NASM64_Function_Meta *meta =
        list_NASM64_Function_Meta_get(gen->functions, i);
    if (STR_EQ(name, meta->name)) {
      return meta;
    }
  }
  return NULL;
}
NASM64_ExprResult *NASM64_generate_expr(NASM64_generator *gen,
                                        NERO_Value* value, char *stream);

NASM64_ExprResult *NASM64_generate_function_call(NASM64_generator *gen,
                                                 struct FCall call) {}

NASM64_ExprResult *NASM64_generate_expr(NASM64_generator *gen,
                                      NERO_Value* value, char *stream) {
//  switch (value.type){
//    case NERO_Int:{
//      NASM64_ExprResult* res = new_expr_result();
//        SB_set(res->result, "%d", value->val.constant_value->value);
//       return res;
//    } break;
//    default:
//      break;
//    }
    return NULL;
}
void NASM64_generate_store(NASM64_generator *gen, NERO_Inst* inst,
                         char *stream) {
			     // const char* name = inst->dst.val.string_value;
  int offset = gen->current_offset++;
  NASM64_ExprResult* res = NASM64_generate_expr(gen, inst->src, stream);
  NASM("    mov [rbp - %d], %s\n", offset, res->result->data);
}

void NASM64_generate_body(NASM64_generator *gen, NERO_Inst** body, int size) {
  int max = size, i = 0;
  while (i < max) {
    NERO_Inst *stmt = body[i];
    switch (stmt->opcode) {
    case NERO_OP_STORE: {
      NASM64_generate_store(gen, stmt, NULL);
    } break;
    default:
      TIX_LOG(stderr, ERROR, "Invalid Statement");
      exit(1);
    }
    ++i;
  }
}

void NASM64_generate_function(NASM64_generator *gen, NERO_Inst* fn) {
  NASM("\n%s:\n", fn->src->val.function_value->name);
  NERO_Inst** body = fn->src->val.function_value->insts;
  int size = fn->src->val.function_value->size;
  NASM64_generate_body(gen, body, size);
  NASM("\n    ret\n");
}
void NASM64_generate_extrn(NASM64_generator *gen, NERO_Inst* inst) {
  switch (inst->src->type) {
  case NERO_OP_FUNCTION: {
    // NASM64_Function_Meta *meta;
    // NASM64_Function_Meta_init(&meta, &ex.symbol.fn);
    // list_NASM64_Function_Meta_add(gen->functions, meta);
    // sbapp(header, "\nextern %s", inst->src->val.string_value);
  } break;
  default:
    break;
  }
}
void NASM64_deinit(NASM64_generator *g) {
  SB_init(&nasm_out);
  sbapp(nasm_out,
        "; ------ this file was generated by the Tix compiler -----\n");
  sbapp(nasm_out, "%s", header->data);
  sbapp(nasm_out, "\nsection .data");
  sbapp(nasm_out, "%s", data->data);
  sbapp(nasm_out, "\nsection .text");
  sbapp(nasm_out, "%s", text->data);
  sbapp(nasm_out, "\nsection .bss");
  sbapp(nasm_out, "\nsection .rodata");
  sbapp(nasm_out, "\nsection .note.GNU-stack");
  SB_free(header);
  SB_free(text);
  SB_free(data);
  SB_free(rodata);
  SB_free(bss);
}
void NASM64_generate(NASM64_generator *n) {
  int max = n->program->count;
  int i = 0;
  while (i < max) {
    NERO_Inst *inst = list_NERO_Inst_get(n->program, i);
    switch (inst->opcode) {
    case NERO_OP_FUNCTION: {
      NASM64_generate_function(n, inst);
    } break;
    case NERO_OP_DECLARE:
      NASM64_generate_extrn(n, inst);
      break;
    default:
      TIX_LOG(stderr, ERROR, "Invalid node");
      exit(1);
    }
    ++i;
  }
}

void nasm_compile(list_NERO_Inst *program, char** source, BuildOptions* opts) {
  NASM64_generator* gen;
  NASM64_init(&gen, program, Source, opts);
  NASM64_generate(gen);
  NASM64_deinit(gen);
  if (opts->stage == ASM) {
    FILE* final_out = fopen(opts->outputfile, "w");
    fprintf(final_out, "%s", nasm_out->data);
    fclose(final_out);
  }
}
