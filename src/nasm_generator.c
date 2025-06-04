
#include "nasm_generator.h"
#include "build/build_opt.h"
#include "context.h"
#include "internals.h"
#include "lexer.h"
#include "node.h"
#include "string_builder.h"
#include "symbol.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *nasm_out;
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
} NASM64_ExprResult;
NASM64_ExprResult *new_expr_result() {
  NASM64_ExprResult *res = NEW(NASM64_ExprResult);
  SB_init(&res->result);
  SB_init(&res->preamble);
  SB_init(&res->opsize);
  res->is_initialized_expr = false;
  res->object_name = (char *)malloc(64);
  res->ty.is_signed = false;
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
const char *NASM64_get_registerv(int *i) { return sysv_regs[(*i)++]; }
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

void NASM64_init(NASM64_generator **n, Program p, char **source,
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
                                        struct Expr *expr, char *stream);

NASM64_ExprResult *NASM64_generate_function_call(NASM64_generator *gen,
                                                 struct FCall call) {
  NASM64_ExprResult *res = new_expr_result();
  // determine callee type
  switch (call.callee->kind) {
  case TEXPR_EXPRIDENT: {
    const char *function_name = call.callee->ident_name;
    NASM64_Function_Meta *meta = NASM64_has_function(gen, function_name);
    if (meta) {
      sbapp(res->preamble, "\n");
      int arg_i = 0;
      int reg_c = 0;

      int arg_max = call.arg_count;
      while (arg_i < arg_max) {
        NASM64_ExprResult *arg_res =
            NASM64_generate_expr(gen, list_Expr_get(call.args, arg_i), NULL);
        if (!Type_match(arg_res->ty,
                        list_Param_get(meta->parameters, arg_i)->type)) {
          printf("Not Param match\n");
          exit(1);
        }
        if (reg_c < 6) {
          Type sib = list_Param_get(meta->parameters, arg_i)->type;
          const char *reg = NASM64_get_registerv(&reg_c);

          printf("reg: %s\n", reg);
          MovOpcode mov =
              MovOpcode_get_movopcode(sib, arg_res->ty.is_signed, arg_res->ty);
          sbapp(res->preamble, "%s", arg_res->preamble->data);
          sbapp(res->preamble, "\n    mov %s, %s", reg, arg_res->result->data);
        } else {
          sbapp(res->preamble, "\n    push %s", arg_res->result->data);
        }
        ++arg_i;
      }
      sbapp(res->preamble, "\n    sub rsp, 16");
      sbapp(res->preamble, "\n    call %s", meta->name);
      sbapp(res->preamble, "\n    add rsp, 16");
      SB_set(res->result, "rax");
    }
  } break;
  default: {
  } break;
  }
  return res;
}

NASM64_ExprResult *NASM64_generate_expr(NASM64_generator *gen,
                                        struct Expr *expr, char *stream) {
  switch (expr->kind) {
  case TEXPR_EXPRINT: {
    int64_t i = expr->int_value;
    NASM64_ExprResult *res = new_expr_result();
    *res->preamble->data = '\0';
    SB_set(res->result, "%d", (int)i);
    res->ty = Type_create_i32();
    res->is_initialized_expr = true;
    return res;
  } break;
  case TEXPR_EXPRSTR: {
    NASM64_ExprResult *res = new_expr_result();
    sbapp(data, "\ngbval%d: db %s, 0", gen->temp_counter,
          NASM64_string_escape(expr->string_value));
    sbapp(res->preamble, "\n    lea rdi, gbval%d", gen->temp_counter++);
    SB_set(res->result, "rdi");
    res->is_initialized_expr = true;
    res->ty = Type_create_i32();
    res->ty.base = TTYPE_U8;
    res->ty.is_ptr = true;
    return res;
  } break;
  case TEXPR_EXPRIDENT: {
    const char *name = expr->ident_name;
    NASM64_ExprResult *res = new_expr_result();
    int max = gen->current_context->symbols->count;
    for (int i = 0; i < max; ++i) {
      Symbol *sym = list_Symbol_get(gen->current_context->symbols, i);
      if (strcmp(sym->name, name) == 0) {
        SB_set(res->result, "[rbp - %d]", sym->offset);
        res->is_initialized_expr = sym->is_init ? true : false;
        res->ty = sym->type;
        return res;
      }
    }
    TIX_LOG(stderr, ERROR, "Use of undeclared variable '%s'", name);
    exit(1);
  } break;
  case TEXPR_BINEXPR: {
    struct BinOp bexpr = expr->binary_op;
    NASM64_ExprResult *res = new_expr_result();
    NASM64_ExprResult *lhs_r = NASM64_generate_expr(gen, bexpr.lhs, stream);
    NASM64_ExprResult *rhs_r = NASM64_generate_expr(gen, bexpr.rhs, stream);
    if (lhs_r->preamble) {
      sbapp(res->preamble, "%s", lhs_r->preamble);
    }
    if (rhs_r->preamble) {
      sbapp(res->preamble, "%s", rhs_r->preamble);
    }
    switch (bexpr.op) {
    case TADD: {
      sbapp(res->preamble, "\n    mov eax, %s", lhs_r->result->data);
      sbapp(res->preamble, "\n    push eax");
      sbapp(res->preamble, "\n    mov ebx, %s", rhs_r->result->data);
      sbapp(res->preamble, "\n    pop eax");
      sbapp(res->preamble, "\n    add eax, ebx\n");
      res->ty = Type_create_i32();
    } break;

    default:
      break;
    }
    SB_set(res->result, "eax");
    res->is_initialized_expr = true;
    return res;
  } break;
  case TEXPR_UNOPEXPR: {
    NASM64_ExprResult *res = new_expr_result();
    switch (expr->unop.op) {
    case TSUB: {
      NASM64_ExprResult *exres =
          NASM64_generate_expr(gen, expr->unop.expr, stream);
      if (expr->unop.expr->kind != TEXPR_EXPRINT) {
        if (exres->preamble) {
          sbapp(res->preamble, "%s", exres->preamble->data);
        }
        sbapp(res->preamble, "\n    mov rax, -%s        ; rax = -%s",

              exres->result->data, exres->result->data);
        SB_set(res->result, "rax");
      } else {
        SB_set(res->result, "-%s", exres->result->data);
        SB_set(res->opsize, "qword");
      }
      res->ty = Type_create_i32();
    } break;
    default:
      break;
    }
    res->is_initialized_expr = true;
    // SB_set(res->result, "rax");
    return res;
  } break;
  case TEXPR_FUNCCALL: {
    NASM64_ExprResult *res = NASM64_generate_function_call(gen, expr->call);
    return res;
  } break;
  default:
    TIX_LOG(stderr, ERROR, "Invalid Expression");
    exit(1);
  }
}
void NASM64_generate_let(NASM64_generator *gen, struct LetStmt *letstmt,
                         char *stream) {
  const char *identifier = letstmt->name;
  if (!NASM64_is_free_symbol_identifier(gen, identifier)) {
    TIX_LOG(stderr, ERROR, "Redefinition of symbol '%s'", identifier);
    exit(1);
  }
  int offset = NASM64_get_next_offset(gen);
  int type_size = NASM64_get_type_size(letstmt->type);
  offset = -NASM64_align(offset, type_size, type_size);
  NASM64_set_next_offset_to_next_multiple_of_4(gen);
  Symbol *sym;
  Symbol_init(&sym, identifier, letstmt->type, offset);
  gen->total_rsp_size += type_size;
  char result[64] = {0};
  NASM64_ExprResult *res;
  if (letstmt->init) {
    res = NASM64_generate_expr(gen, letstmt->init, result);
    if (!res->is_initialized_expr) {
      TIX_LOG(stderr, ERROR,
              "Cannot initialize a value with an uninitialized value");
      exit(1);
    }
    MovOpcode mov = MovOpcode_get_movopcode(letstmt->type, false, res->ty);
    // MovOpcode mov = MOVQ;
    char *opsize = "";
    if (res->preamble) {
      tstrcatf(stream, "\n%s", res->preamble->data);
    }
    if (res->opsize->len > 0) {
      opsize = res->opsize->data;
    }

    tstrcatf(stream, "\n    mov %s [rbp - %d], %s", opsize, offset, res->result->data);
    sym->is_init = true;
  } else {
    sym->is_init = false;
  }
  if (res->ty.is_signed) {
    sym->type.is_signed = true;
  }
  list_Symbol_add(gen->current_context->symbols, sym);
  gen->current_offset = -offset;
}
void NASM64_generate_body(NASM64_generator *gen, Stmt *stmt, char *stream) {
  list_Stmt *statements = stmt->stmt.statements;
  int max = statements->count, i = 0;
  while (i < max) {
    Stmt *stmt = list_Stmt_get(statements, i);
    switch (stmt->kind) {
    case TSTMT_LET: {
      NASM64_generate_let(gen, &stmt->stmt.letstmt, stream);
    } break;
    case TSTMT_EXPR: {
      NASM64_ExprResult *res =
          NASM64_generate_expr(gen, stmt->stmt.expr, stream);
      tstrcatf(stream, "%s", res->preamble->data);
    } break;
    default:
      TIX_LOG(stderr, ERROR, "Invalid Statement");
      exit(1);
    }
    ++i;
  }
}

void NASM64_generate_function(NASM64_generator *gen, struct FnStmt *fn) {
  const char *function_name;
  bool is_extern = false;
  NASM("\n; ----- %s ---- ", fn->name);
  if (STR_EQ(fn->name, "main")) {
    function_name = "t_main";
    is_extern = true;
  } else {
    function_name = fn->name;
  }
  if (is_extern)
    NASM("\nextern t_main");
  if (!NASM64_is_free_symbol_identifier(gen, function_name)) {
    TIX_LOG(stderr, ERROR, "Redefinition of symbol'%s'", function_name);
    exit(1);
  }
  NASM("\n%s:", function_name);
  NASM64_Context *ctx;
  NASM64_Context_init(&ctx, gen->current_context);
  NASM64_enter_context(gen, ctx);
  if (fn->body) {
    NASM("\n    push rbp");
    NASM("\n    mov rbp, rsp");
    gen->track_rsp_allocations = true;
    char *stream = malloc(1024);
    NASM64_generate_body(gen, fn->body, stream);
    if (gen->total_rsp_size > 0)
      NASM("\n    sub rsp, %d", gen->total_rsp_size);
    NASM("%s", stream);
    if (gen->total_rsp_size > 0)
      NASM("\n    add rsp, %d", gen->total_rsp_size);
    NASM("\n    leave");
  }
  NASM64_Function_Meta *fnmeta;
  NASM64_Function_Meta_init(&fnmeta, fn);
  list_NASM64_Function_Meta_add(gen->functions, fnmeta);

  NASM("\n    ret");
  gen->total_rsp_size = 0;
  NASM64_exit_context(gen, ctx);
}
void NASM64_generate_extrn(NASM64_generator *gen, struct ExternStmt ex) {
  switch (ex.kind) {
  case EXTERN_FN: {
    NASM64_Function_Meta *meta;
    NASM64_Function_Meta_init(&meta, &ex.symbol.fn);
    list_NASM64_Function_Meta_add(gen->functions, meta);
    sbapp(header, "\nextern %s", ex.symbol.fn.name);
  } break;
  default:
    break;
  }
}
void NASM64_deinit(NASM64_generator *g) {
  nasm_out = fopen(buildOptions->outputfile, "w");
  fprintf(nasm_out,
          "; ------ this file was generated by the Tix compiler -----\n");
  fprintf(nasm_out, "%s", header->data);
  fprintf(nasm_out, "\nsection .data");
  fprintf(nasm_out, "%s", data->data);
  fprintf(nasm_out, "\nsection .text");
  fprintf(nasm_out, "%s", text->data);
  fprintf(nasm_out, "\nsection .bss");
  fprintf(nasm_out, "\nsection .rodata");
  fprintf(nasm_out, "\nsection .note.GNU-stack");
  SB_free(header);
  SB_free(text);
  SB_free(data);
  SB_free(rodata);
  SB_free(bss);
}
void NASM64_parse_item(NASM64_generator *gen, Item *item) {
  switch (item->kind) {
  case ITEM_FN: {
    NASM64_generate_function(gen, &item->fn);
  } break;
  case ITEM_EXTRN: {
    NASM64_generate_extrn(gen, item->extrn);
  } break;
  default:
    TIX_LOG(stderr, ERROR, "Invalid Item");
    exit(1);
  }
}

void NASM64_generate(NASM64_generator *n) {
  int max = n->program.count;
  int i = 0;
  while (i < max) {
    Node *node = n->program.nodes[i];
    switch (node->type) {
    case TNODE_ITEM: {
      NASM64_parse_item(n, node->node.item);
    } break;
    default:
      TIX_LOG(stderr, ERROR, "Invalid node");
      exit(1);
    }
    ++i;
  }
}
