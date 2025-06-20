#include "nero.h"
#include "../internals.h"
#include <stdio.h>
#include <stdlib.h>


list_NERO_Inst* Nero_parse(Program *program);
const char* NERO_DEBUG(NERO_Inst* inst) {
  switch (inst->opcode) {
    case NERO_OP_CONST:{
      char buf[12] = {0};
      sprintf(buf, "%d", inst->src->val.constant_value->value);
      return strdup(buf);
    } break;
    default:
      return "<>";
  }
  return NULL;
}

NERO_Inst* NERO_parse_expr(Expr* expr) {
  NERO_Inst* inst = NEW(NERO_Inst);
  switch (expr->kind) {
  case TEXPR_EXPRINT:
      inst->opcode = NERO_OP_CONST;
      inst->src = NeroInt(NERO_new_const(expr->int_value));
      break;
  case TEXPR_BINEXPR:
      // derive:     let num: i32 = 1 + 1;
      // RES %num, DWORD
      // LOAD %tmp, 1
      // ADD %tmp, 1
      // STORE %num, %tmp
      const char* tmp = "%tmp";
      NERO_Inst* lhs = NERO_parse_expr(expr->binary_op.lhs);
      NERO_Inst* load = NERO_make_inst(NERO_OP_LOAD, NERO_Value_from_load(tmp, lhs), NeroNull());
      break;
  default:
      break;
  }
  return inst;
}

list_NERO_Inst* NERO_parse_body(Stmt* body) {
  list_NERO_Inst* insts;
  list_NERO_Inst_init(&insts);
  if (body) {
      int max = body->stmt.statements->count;
      int i = 0;
      printf("        Body:\n");
      while (i < max) {
	  Stmt* stmt = list_Stmt_get(body->stmt.statements, i);
	  switch (stmt->kind) {
	  case TSTMT_LET:{
              struct LetStmt ls = stmt->stmt.letstmt;
	      NERO_Inst* reserve = NERO_make_inst(NERO_OP_RES, NERO_Value_from_reserve(ls.name, ls.type.size_in_bytes), NeroNull());
	      NERO_Inst* store   = NERO_make_inst(NERO_OP_STORE, NERO_Value_from_storage(ls.name, NERO_parse_expr(ls.init)), NeroNull());
              const char* name = ls.name;
              const char* type = Type_toraw(&ls.type);
              printf("        STORE %s %s\n", name, type);
              list_NERO_Inst_add(insts, reserve);
	      list_NERO_Inst_add(insts, store);
	  } break;
          default:
            break;
	}
	i++;
    }
  }
  return insts;
}

NERO_Inst* NERO_parse_fn(struct FnStmt fn) {
    NERO_Inst* inst = NEW(NERO_Inst);   
    inst->opcode = NERO_OP_FUNCTION;
    printf("    Function: %s\n", fn.name);
    list_NERO_Inst* insts = NERO_parse_body(fn.body);
    NERO_Function *f = NEW(NERO_Function);
    
    f->name=  fn.name;
    f->insts = insts->data;
    f->size =  insts->count;
    int param_count = fn.param->count;
    NERO_Arg** args = malloc(param_count*sizeof(NERO_Arg));
    int i = 0;
    while(i < param_count) {
	NERO_Arg* arg = NEW(NERO_Arg);
	Param* param = list_Param_get(fn.param, i);
	arg->name = param->name;
	arg->type = param->type;
	args[i] = arg;
	i++;
    }
    f->arguments = args;
    f->argc = param_count;
    inst->src = NERO_Value_from_fn(f);
    printf("    Function End\n");
    return inst;
}

NERO_Inst* NERO_parse_item(Item* item) {
  NERO_Inst* inst = NEW(NERO_Inst);
  switch(item->kind) {
  case ITEM_FN:{
	inst = NERO_parse_fn(item->fn);
    } break;
    case ITEM_EXTRN:{
	NERO_Declare* extrn = NEW(NERO_Declare);
        inst->opcode = NERO_OP_DECLARE;
        printf("    Extern: %s\n", item->extrn.symbol.fn.name);
	
	switch(item->extrn.kind) {
	case EXTERN_FN:
	    extrn->type  = NERO_DECLARE_FUNCTION;
	    extrn->value = NERO_parse_fn(item->extrn.symbol.fn);
	    inst->src    = NERO_Value_from_declaration(extrn);
	    break;
	default:
	    break;
	}
    } break;
    default:
      break;
  }
  return inst;
}

list_NERO_Inst* Nero_parse(Program *program) {
  list_NERO_Inst* insts;
  list_NERO_Inst_init(&insts);
  int max = program->count;
  int i = 0;
  while (i < max) {
    Node* node = program->nodes[i];
    switch (node->type) {
      case TNODE_ITEM:
        printf("Top level item\n");
        list_NERO_Inst_add(insts, NERO_parse_item(node->node.item));
        break;
      default:
        printf("Unexpected Node\n");
        break;
    }
    i++;
  }
  return insts;
}
