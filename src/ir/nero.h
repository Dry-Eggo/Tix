#pragma once
#include "../string_builder.h"
#include "../lists/lists.h"
#include "../node.h"

// NERO:              Intermediate Representation of Tix source code

typedef enum {
    // Memory
    NERO_OP_CONST,                       // CONST 40
    NERO_OP_RES,
    NERO_OP_STORE,                       // STORE %tmp, CONST 40
    NERO_OP_LOAD,                        // LOAD %dst, %source
    
    // Arithmetics
    NERO_OP_ADD,
    NERO_OP_SUB,
    NERO_OP_MUL,
    NERO_OP_DIV,

    // Miscellenous
    NERO_OP_FUNCTION,                    // function %main:...
    NERO_OP_DECLARE,                     // DECLARE @printf
    NERO_OP_CALL,                        // call @printf(%fmt, %msg)
    NERO_OP_LABEL,
} NERO_Opcode;

typedef enum {
  NERO_Ptr,
  NERO_Byte,
  NERO_Word,
  NERO_Double_word,
  NERO_Quad_word,
  NERO_Null,  // To null out unused operand in single instruction operands
} NERO_Type;

typedef enum {
    NERO_DECLARE_FUNCTION,
    NERO_DECLARE_VARIABLE,
} NERO_DECLARE_TYPE;

struct NERO_Inst;

typedef struct {
  const char*             name;
  NERO_Type                    type;
} NERO_Arg;


struct NERO_Value;

typedef struct  {
  const char*             name;
  struct NERO_Inst**      insts;
  int                     size;

  NERO_Arg**              arguments;
  int                     argc;
} NERO_Function;

typedef struct {
    const char*           name;
    struct NERO_Inst**    args;
    int                   argc;
} NERO_Call;

typedef struct {
  NERO_DECLARE_TYPE type;
  struct NERO_Inst* value;
} NERO_Declare;

typedef struct {
  int                     value;
} NERO_Const;


/* STORE Instruction stores a value provided to it in an address */
typedef struct {
  const char*             label;
  struct NERO_Inst*       value;
} NERO_Store;

/* LOAD Instruction is a version of STORE that stores a value in a non-memory location */
typedef struct {
  const char*             label;
  struct NERO_Inst*       value;
} NERO_Load;

/* RES Instruction allocates memory for a variable on the stack */
typedef struct {
  const char*             label;
  NERO_Type               size;
} NERO_Res;

typedef struct {
    const char*           label;
} NERO_Label;

// lhs must be a label: ADD %foo, 3. this is because it is also where the result of the Expression will be stored
typedef struct {
    NERO_Opcode op;
    struct NERO_Value*           lhs;
    struct NERO_Inst*           rhs;
} NERO_Ari;               // Arithmetics

typedef struct NERO_Value {
  NERO_Type               type;
  union {
    NERO_Store*           storage_value;
    NERO_Const*           constant_value;
    NERO_Declare*         declaration_value;
    NERO_Function *       function_value;
    NERO_Ari*             ari_value;
    NERO_Label*           label_value;
    NERO_Res*             reserve;
    NERO_Load*            load;
    NERO_Call*            call;
  } val;
} NERO_Value;

typedef struct NERO_Inst {
  NERO_Opcode      opcode;
  NERO_Value*       src;
  NERO_Value*       dst;
} NERO_Inst;

// function  %main:           -> NERO_Inst(NERO_FUNCTION, NERO_Value(NERO_Label, "%main"));
// STORE %tmp, CONST 40   -> NERO_Inst(NERO_STORE,    NERO_Value(NERO_Label, "%tmp"), NERO_Value(NERO_Int(40)))

TIX_DYN_LIST(NERO_Inst, NERO_Inst);

static list_NERO_Inst* glob_program;
NERO_Inst*      NERO_make_inst(NERO_Opcode op, NERO_Value* src, NERO_Value* dst);
NERO_Value*     NERO_Value_from_fn(NERO_Function* fn);
NERO_Value*     NERO_Value_from_reserve(const char*, int);
NERO_Value*     NERO_Value_from_storage(const char*, NERO_Inst*);
NERO_Value*     NERO_Value_from_load(const char*, NERO_Inst*);
NERO_Value*     NERO_Value_from_declaration(NERO_Declare* declaration);
NERO_Value*     NERO_Value_from_ari(NERO_Opcode, NERO_Value*, NERO_Inst*);
NERO_Value*     NERO_Value_from_label(const char* label);
NERO_Value*     NERO_Value_from_call(const char* callee, NERO_Inst** args, int argc);
NERO_Value*     NeroNull();
NERO_Value*     NeroInt(NERO_Const* i);
NERO_Const*     NERO_new_const(int i);
list_NERO_Inst* Nero_parse(Program *program);
NERO_Inst*      NERO_parse_item(Item *);
list_NERO_Inst* NERO_parse_body(Stmt *);
NERO_Inst*     NERO_parse_expr(Expr* expr, list_NERO_Inst*);
