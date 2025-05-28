#ifndef TIX_IR
#define TIX_IR

#include <stdio.h>

/*
 * Base for the Tix Textual Immediate Representation
 */
#include "../node.h"
#include <stdint.h>
typedef enum {
  TIR_CALL,
  TIR_LOAD,
  TIR_FUNC,
  TIR_STORE,
  TIR_CMP,
  TIR_JMP
} TIR_Opcode;

typedef enum {
  TIR_STRINGLIT,
  TIR_INTLIT,
  TIR_INST,
} TIR_ExprKind;
struct TIR_Instruction;

typedef enum { TIR_I8, TIR_I16, TIR_I32, TIR_I64, TIR_PTR, TIR_STR } TIR_Type;

typedef struct {
  TIR_ExprKind kind;
  TIR_Type type;
  union {
    struct TIR_Instruction *inst; // e.g: '%x = load (i32) call %foo (str) "Tix
    const char *ident_value;      // e.g: %foo, %bar, %baz
    int64_t int_val;              // e.g: 30, 40, 1, 6
  };
} TIR_Expr;

typedef struct TIR_Instruction {
  TIR_Opcode op;
  TIR_Expr *operand_1;
  TIR_Expr *operand_2;
} TIR_Instruction; /* e.g: load (i32) imm 30, (i32) %1 */

typedef struct IR_list {
  TIR_Instruction **insts;
  int count;
  int cap;
} IR_list;

typedef struct TIR_Value {
  const char *name;
  TIR_Type *type;
} TIR_Value;
typedef struct Node_Walker {
  IR_list *list;
  Program *program;
  int pos;
  int max;
} Node_Walker;
typedef struct TIR_Block TIR_Block;
typedef struct TIR_Function {
  const char *name;
  TIR_Block *entry;
  TIR_Type type;
} TIR_Function;

typedef struct TIR_SymbolTable {
  TIR_Value **symbol;
  int max;
  int index;
} TIR_SymbolTable;

typedef struct TIR_Module {
  const char *name;
  TIR_SymbolTable *globals;
  struct TIR_Module *parent;
} TIR_Module;

typedef struct TIR_Block {
  const char *label;
  IR_list *inst;
  const char *exit_label;
  TIR_Module *context;
} TIR_Block;

void Env_init();
void ir_write(const char *, ...);
void Env_close();
void Node_init(Node_Walker *, Program *);
void IR_list_init(IR_list *);
void IR_list_add(IR_list *, TIR_Instruction *);
void IR_load(TIR_Value *, TIR_Expr *);
void TIR_Value_init(const char *name, TIR_Type type);
TIR_Type TIR_Type_get_i32();
TIR_Type TIR_Type_get_constant_i32();
void IR_Function(TIR_Function *, const char *name, TIR_Block *, TIR_Type type);
void IR_Function_entry(TIR_Function *, TIR_Block *);
Node *Node_next(Node_Walker *);
void Node_add_inst(Node_Walker *, TIR_Instruction *);
void Node_generate(Node_Walker *);
#endif
