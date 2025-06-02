#include "../internals.h"
#include "ir_gen.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TIR_MAX_ENTRIES 1024
static FILE *irfile;
static TIR_Module *__main;
static TIR_SymbolTable *TIR_CURRENT_CONTEXT = NULL;
static const char *TIR_Entry_list[TIR_MAX_ENTRIES];
static size_t TIR_Entry_list_index = 0;
void tlog(const char *msg, ...) {
  va_list arg;
  va_start(arg, msg);
  printf("[Tir] ");
  vprintf(msg, arg);
  printf("\n");
  va_end(arg);
}
void Env_init() {
  // IR_Symbols(TIR_CURRENT_CONTEXT, NULL);
  tlog("Initializing Tix Module");
  IR_Module(&__main, "tixc_main"); /* global symbols */
  tlog("Created 'tixc_main' Module");
  IR_EnterContext(__main->globals);
  irfile = fopen("t.tir", "w");
  if (!irfile) {
    fprintf(stderr, "Fatal: Unable to open Ir file");
    exit(1);
  }
}

/*----------------IR_Entry-------------------*/
void TIR_Register_Entry(const char *name) {
  TIR_Entry_list[TIR_Entry_list_index++] = name;
}

int TIR_Entry_exists(const char *name) {
  int found = -1;
  for (int i = 0; i < TIR_Entry_list_index; i++) {
    const char *entry = TIR_Entry_list[i];
    if (strcmp(name, entry) == 0)
      found = 0;
    break;
  }
  return found;
}

void IR_EnterContext(TIR_SymbolTable *s) {
  tlog("Entered Scope");
  s->parent = TIR_CURRENT_CONTEXT;
  tlog("Entered Scope");
  TIR_CURRENT_CONTEXT = s;
}
void IR_ExitContext() { TIR_CURRENT_CONTEXT = TIR_CURRENT_CONTEXT->parent; }
/*----------------IR_Module-------------------*/
void IR_Module(TIR_Module **m, const char *name) {
  tlog("Creating Module (%s)", name);
  *m = (TIR_Module *)malloc(sizeof(TIR_Module));
  (*m)->name = name;
  IR_SymbolTable_init(&(*m)->globals, TIR_CURRENT_CONTEXT);
  list_TIR_Function_init(&(*m)->functions);
}
void IR_Module_dump(TIR_Module **m, FILE *stream) {
  if (stream == NULL) {
    stream = irfile;
  }
  int fi = (*m)->functions->count;
  for (int i = 0; i < fi; ++i) {
    TIR_Function *fn = list_TIR_Function_get((*m)->functions, i);
    if (fn)
      fprintf(stream, "%s", IR_Function_tostr(fn));
    if (fn->entry) {
      int ei = fn->entry->context->symbol->count;
      for (int i = 0; i < ei; ++i) {
        TIR_Value *v = list_TIR_Value_get(fn->entry->context->symbol, i);
        fprintf(stream, "\n%s", IR_Value_toraw(v));
      }
    }
    fprintf(stream, "end");
  }
}
TIR_Module *IR_Get_mainmodule(void) { return __main; }
/*----------------IR_Symbol-------------------*/
void IR_SymbolTable_init(TIR_SymbolTable **sym, TIR_SymbolTable *parent) {
  tlog("Creating Symbol Table");
  *sym = (TIR_SymbolTable *)malloc(sizeof(TIR_SymbolTable));
  list_TIR_Value_init(&(*sym)->symbol);
  tlog("Allocated Symbol Table");
  (*sym)->parent = parent;
}
/*
  Searches a symbol table for a specific symbol.
  returns NULL is not found else returns pointer to value
*/
TIR_Value *IR_SymbolTable_get(TIR_SymbolTable **sym, const char *query) {
  size_t max = (*sym)->symbol->count;
  for (int i = 0; i < max; ++i) {
    TIR_Value *v = list_TIR_Value_get((*sym)->symbol, i);
    if (strcmp(v->name, query) == 0)
      return v;
  }
  return NULL;
}
bool IR_SymbolTable_store(TIR_SymbolTable **sym, TIR_Value *v) {
  return (list_TIR_Value_add((*sym)->symbol, v) == 0) ? true : false;
}
/*----------------IR_Function-------------------*/
void IR_Function(TIR_Function **fn, TIR_Module **t, const char *name,
                 TIR_Block *eb, TIR_Type type) {
  *fn = malloc(sizeof(TIR_Function));
  if (fn) {
    list_TIR_Function_add((*t)->functions, (*fn));
  }
  (*fn)->name = name;
  (*fn)->entry = eb;
  (*fn)->type = type;
}
const char *IR_Function_tostr(TIR_Function *f) {
  char text[1024] = {0};
  strcat(text, "\nfunction (");
  strcat(text, TIR_Type_tostr(f->type));
  strcat(text, ") %");
  strcat(text, f->name);
  return strdup(text);
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
  printf("Wrote to irfile\n");
}
/*----------------IR_Type-------------------*/
const char *TIR_Type_tostr(TIR_Type t) {
  switch (t) {
  case TIR_I32:
    return strdup("i32\0");
  default:
    return strdup("ptr\0");
  }
}
TIR_Type *IR_Type_dup(TIR_Type t) {
  TIX_LOG(stdout, INFO, "Duplicated type");
  TIR_Type *ty = malloc(sizeof(TIR_Type));
  *ty = t;
  return ty;
}
TIR_Type TIR_Type_get_i32() { return TIR_I32; }
TIR_Type IR_Type_from_type(Type *t) {
  switch (t->base) {
  case TTYPE_I8:
    return TIR_I8;
  case TTYPE_I16:
    return TIR_I16;
  case TTYPE_I32:
    return TIR_I32;
  case TTYPE_I64:
    return TIR_I64;
  }
}

/*----------------IR_Value-------------------*/
void IR_Value(TIR_Value **value, const char *name, TIR_Type type,
              TIR_Value *init, TIR_Block **ctx) {
  *value = (TIR_Value *)malloc(sizeof(TIR_Value));
  (*value)->name = name;
  (*value)->type = IR_Type_dup(type);
  if (init) {
    if (init->is_int) {
      (*value)->is_int = true;
      (*value)->val.int_value = init->val.int_value;
    } else {
      (*value)->is_str = true;
      (*value)->val.str_value = strdup(init->val.str_value);
    }
  }
}
TIR_Value *IR_Value_Get_Constant_I32(int64_t val) {
  TIR_Value *value = (TIR_Value *)malloc(sizeof(TIR_Value));
  value->type = (TIR_Type *)malloc(sizeof(TIR_Type));
  *value->type = TIR_Type_get_i32();
  value->val.int_value = val;
  value->is_int = true;
  return value;
}

void IR_Value_Store(TIR_Value **v, TIR_Block **ctx) {
  IR_SymbolTable_store(&(*ctx)->context, (*v));
  TIX_LOG(stdout, INFO, "Stored Value");
}
const char *IR_Value_toraw(TIR_Value *v) {
  char buf[1024] = {0};
  strcat(buf, "(");
  strcat(buf, TIR_Type_tostr(*v->type));
  strcat(buf, "), %");
  strcat(buf, v->name);
  buf[1023] = '\0';
  return strdup(buf);
}
/*----------------IR_Block-------------------*/
void IR_Block(TIR_Block **b, const char *entry, const char *exitl) {
  *b = (TIR_Block *)malloc(sizeof(TIR_Block));
  if (!entry) {
    TIX_LOG(stderr, ERROR, "Every Block must have an entry label");
    exit(1);
  }
  (*b)->label = strdup(entry);
  (*b)->exit_label = (exitl == NULL) ? "" : exitl;
  IR_SymbolTable_init(&(*b)->context, NULL);
}

void Env_close() { fclose(irfile); }
