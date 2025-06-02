#include "nasm_generator.h"
#include "build/build_opt.h"
#include "context.h"
#include "internals.h"
#include "lexer.h"
#include "node.h"
#include "symbol.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *nasm_out;
static char **Source;
#define NASM(stream, ...) fprintf(nasm_out, stream, ##__VA_ARGS__);
#define STR_EQ(s1, s2) strcmp(s1, s2) == 0

typedef struct {
  Type ty;
  char *preamble, *result;
  bool is_initialized_expr;
} NASM64_ExprResult;
NASM64_ExprResult *new_expr_result() {
  NASM64_ExprResult *res = NEW(NASM64_ExprResult);
  res->preamble = (char *)malloc(1024);
  res->result = (char *)malloc(1024);
  res->is_initialized_expr = false;
  return res;
}
typedef enum MovOpcode { MOVB, MOVD, MOVW, MOVSX, MOVZX, MOVQ } MovOpcode;
const char *MovOpcode_tostr(MovOpcode op) {
  switch (op) {
  case MOVB:
    return strdup("mov byte");
  case MOVD:
    return strdup("mov dword");
  case MOVQ:
    return ("mov qword");
  case MOVSX:
    return strdup("movsx");
  case MOVZX:
    return strdup("movzx");
  case MOVW:
    return strdup("mov word");
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
  if (NASM64_Context_rsearch(gen->current_context, ident)) {
    return true;
  }
  int max = gen->functions->count;
  for (int c = 0; c < max; c++) {
    if (strcmp(list_NASM64_Function_Meta_get(gen->functions, c)->name, ident) ==
        0) {
      return false;
    }
  }
  return true;
}
MovOpcode MovOpcode_get_movopcode(Type type, bool is_signed, Type dst) {
  if (type.size_in_bytes == dst.size_in_bytes) {
    switch (type.size_in_bytes) {
    case 1:
      return MOVB;
    case 2:
      return MOVW;
    case 4:
      return MOVD;
    case 8:
      return MOVQ;
    }
  }
  if (type.size_in_bytes < dst.size_in_bytes) {
    if (is_signed)
      return MOVSX;
    return MOVZX;
  }
  return MOVQ;
}

int NASM64_get_next_offset(NASM64_generator *gen) {
  return gen->current_offset;
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
  (*n)->current_offset = 1;
  (*n)->rsp_loc = 0;
  (*n)->total_rsp_size = 0;
  (*n)->track_rsp_allocations = false;
  NASM64_Context_init(&(*n)->global_context, NULL);
  (*n)->current_context = (*n)->global_context;
  list_NASM64_Function_Meta_init(&(*n)->functions);
  if (!b->outputfile) {
    b->outputfile = "t.s";
  }
  nasm_out = fopen(b->outputfile, "w");
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
  switch (ty.base) {
  case TTYPE_I32:
    return 4;
  default:
    TIX_LOG(stderr, ERROR, "Invalid Type");
    exit(1);
  }
}

NASM64_ExprResult *NASM64_generate_expr(NASM64_generator *gen,
                                        struct Expr *expr, char *stream) {
  switch (expr->kind) {
  case TEXPR_EXPRINT: {
    int64_t i = expr->int_value;
    NASM64_ExprResult *res = new_expr_result();
    *res->preamble = '\0';
    sprintf(res->result, "%d", (int)i);
    res->ty = Type_create_i32();
    res->is_initialized_expr = true;
    return res;
  } break;
  case TEXPR_EXPRIDENT: {
    const char *name = expr->ident_name;
    NASM64_ExprResult *res = new_expr_result();
    int max = gen->current_context->symbols->count;
    for (int i = 0; i < max; ++i) {
      Symbol *sym = list_Symbol_get(gen->current_context->symbols, i);
      if (strcmp(sym->name, name) == 0) {
        sprintf(res->preamble, "\n    mov rbx, [rbp - %d]", sym->offset);
        sprintf(res->result, "rbx");
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
      tstrcatf(res->preamble, "%s", lhs_r->preamble);
    }
    if (rhs_r->preamble) {
      tstrcatf(res->preamble, "%s", rhs_r->preamble);
    }
    switch (bexpr.op) {
    case TADD: {
      tstrcatf(res->preamble, "\n    mov rax, %s", lhs_r->result);
      tstrcatf(res->preamble, "\n    push rax");
      tstrcatf(res->preamble, "\n    mov rbx, %s", rhs_r->result);
      tstrcatf(res->preamble, "\n    pop rax");
      tstrcatf(res->preamble, "\n    add rax, rbx\n");
    } break;
    default:
      break;
    }
    sprintf(res->result, "rax");
    res->is_initialized_expr = true;
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
    TIX_LOG(stderr, ERROR, "Redefinition of symbol'%s'", identifier);
    exit(1);
  }
  int offset = NASM64_get_next_offset(gen);
  int type_size = NASM64_get_type_size(letstmt->type);
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
    // MovOpcode mov = MovOpcode_get_movopcode(letstmt->type, false, res.ty);
    MovOpcode mov = MOVQ;
    if (res->preamble) {
      tstrcatf(stream, "\n%s", res->preamble);
    }
    tstrcatf(stream, "\n    %s [rbp - %d], %s", MovOpcode_tostr(mov), offset,
             res->result);
    sym->is_init = true;
  } else {
    TIX_LOG(stdout, INFO, "Not Initialized");
    sym->is_init = false;
  }
  list_Symbol_add(gen->current_context->symbols, sym);
  TIX_LOG(stdout, INFO, "No. of symbols: %d",
          gen->current_context->symbols->count);
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
    default:
      tix_error(stmt->span, "Invalid Statement", Source, NULL);
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
    NASM("\n    push rsp");
    NASM("\n    mov rbp, rsp");
    gen->track_rsp_allocations = true;
    char *stream = malloc(1024);
    NASM64_generate_body(gen, fn->body, stream);
    NASM("\n    sub rsp, %d", gen->total_rsp_size);
    NASM("%s", stream);
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
void NASM64_deinit(NASM64_generator *g) { NASM("\nsection .note.GNU-stack"); }
void NASM64_parse_item(NASM64_generator *gen, Item *item) {
  switch (item->kind) {
  case ITEM_FN: {
    NASM64_generate_function(gen, &item->fn);
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
