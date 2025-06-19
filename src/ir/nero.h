#pragma once
#include "../string_builder.h"
#include "../lists/lists.h"
#include "../node.h"

// NERO:              Intermediate Representation of Tix source code

typedef enum {
  NERO_OP_CONST,                       // CONST 40
  NERO_OP_STORE,                       // STORE %tmp, CONST 40
  NERO_OP_FUNCTION,                    // func %main:...
  NERO_OP_LOAD,                        // LOAD %dst, %source
  NERO_OP_DECLARE,                     // DECLARE printf
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
  Type                    type;
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
  NERO_DECLARE_TYPE type;
  struct NERO_Inst* value;
} NERO_Declare;

typedef struct {
  int                     value;
} NERO_Const;

typedef struct {
  const char*             label;
  struct NERO_Inst*      value;
} NERO_Store;

typedef struct NERO_Value {
  NERO_Type               type;
  union {
    NERO_Store*           storage_value;
    NERO_Const*           constant_value;
    NERO_Declare*         declaration_value;
    NERO_Function *       function_value;
  } val;
} NERO_Value;

typedef struct NERO_Inst {
  NERO_Opcode      opcode;
  NERO_Value*       src;
  NERO_Value*       dst;
} NERO_Inst;

// func  %main:           -> NERO_Inst(NERO_FUNCTION, NERO_Value(NERO_Label, "%main"));
// STORE %tmp, CONST 40   -> NERO_Inst(NERO_STORE,    NERO_Value(NERO_Label, "%tmp"), NERO_Value(NERO_Int(40)))

TIX_DYN_LIST(NERO_Inst, NERO_Inst);

NERO_Inst*      NERO_make_inst(NERO_Opcode op, NERO_Value* src, NERO_Value* dst);
NERO_Value*     NERO_Value_from_fn(NERO_Function* fn);
NERO_Value*     NERO_Value_from_storage(NERO_Store* storage);
NERO_Value*     NERO_Value_from_declaration(NERO_Declare* declaration);
NERO_Value      NeroNull();
NERO_Value*     NeroInt(NERO_Const* i);
NERO_Const*     NERO_new_const(int i);
list_NERO_Inst* Nero_parse(Program *program);
NERO_Inst*      NERO_parse_item(Item *);
list_NERO_Inst* NERO_parse_body(Stmt *);
NERO_Inst*      NERO_parse_expr(Expr* expr);
