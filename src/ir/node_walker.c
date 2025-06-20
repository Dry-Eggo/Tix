#include "nero.h"
#include "../internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

// Not part of Nero Api
NERO_Type NERO_Type_from_t(Type t) {
    // TODO: Add all types
    if (t.base == TI32) return NERO_Double_word;
}

// @param body is used for binary operations. set to NULL is not used
NERO_Inst* NERO_parse_expr(Expr* expr, list_NERO_Inst* body) {
  NERO_Inst* inst = NEW(NERO_Inst);
  switch (expr->kind) {
  case TEXPR_EXPRINT:
      inst->opcode = NERO_OP_CONST;
      inst->src = NeroInt(NERO_new_const(expr->int_value));
      break;
  case TEXPR_EXPRIDENT:
      inst = NERO_make_inst(NERO_OP_LABEL, NERO_Value_from_label(expr->ident_name), NeroNull());
      break;
  case TEXPR_BINEXPR:{
      // derive:     let num: i32 = 1 + 1;
      // RES %num, DWORD
      // LOAD %tmp, 1
      // ADD %tmp, 1
      // STORE %num, %tmp
      const char* tmp = "tmp";
      NERO_Inst* lhs = NERO_parse_expr(expr->binary_op.lhs, NULL);
      NERO_Inst* rhs = NERO_parse_expr(expr->binary_op.rhs, NULL);
      NERO_Inst* load = NERO_make_inst(NERO_OP_LOAD, NERO_Value_from_load(tmp, lhs), NeroNull());
      list_NERO_Inst_add(body, load);
      switch (expr->binary_op.op) {
        case TADD:{
            NERO_Inst* add = NEW(NERO_Inst);
	    add = NERO_make_inst(NERO_OP_ADD, NERO_Value_from_ari(NERO_OP_ADD, NERO_Value_from_label(tmp), rhs), NeroNull());
	    list_NERO_Inst_add(body? body: glob_program, add);	    
        } break;
        default:
          break;
      }
      inst = NERO_make_inst(NERO_OP_LABEL, NERO_Value_from_label(tmp), NeroNull());
  } break;
case TEXPR_FUNCCALL:
    struct FCall fc = expr->call;
    list_NERO_Inst* args;
    list_NERO_Inst_init(&args);
    int max = fc.args->count;
    int i   = 0;
    while(i < max) {
	struct Expr* expr = list_Expr_get(fc.args, i);
	list_NERO_Inst_add(args, NERO_parse_expr(expr, NULL));
	++i;
    }
    if (fc.callee->kind == TEXPR_EXPRIDENT) {
	inst  = NERO_make_inst(NERO_OP_CALL, NERO_Value_from_call(fc.callee->ident_name, args->data, args->count), NeroNull());
    }
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
      while (i < max) {
	  Stmt* stmt = list_Stmt_get(body->stmt.statements, i);
	  switch (stmt->kind) {
	  case TSTMT_LET:{
              struct LetStmt ls = stmt->stmt.letstmt;
	      NERO_Inst* reserve = NERO_make_inst(NERO_OP_RES, NERO_Value_from_reserve(ls.name, ls.type.size_in_bytes), NeroNull());
	      NERO_Inst* store   = NERO_make_inst(NERO_OP_STORE, NERO_Value_from_storage(ls.name, NERO_parse_expr(ls.init, insts)), NeroNull());
              const char* name = ls.name;
              const char* type = Type_toraw(&ls.type);
              list_NERO_Inst_add(insts, reserve);
	      list_NERO_Inst_add(insts, store);
	  } break;
      case TSTMT_EXPR:
	  list_NERO_Inst_add(insts, NERO_parse_expr(stmt->stmt.expr, insts));
	  break;
      default:
	  assert(0 && "Not Yet Implemented");
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
	arg->type = NERO_Type_from_t(param->type);
	args[i] = arg;
	i++;
    }
    f->arguments = args;
    f->argc = param_count;
    inst->src = NERO_Value_from_fn(f);
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
  list_NERO_Inst_add(glob_program, inst);
  return inst;
}

list_NERO_Inst* Nero_parse(Program *program) {
  list_NERO_Inst_init(&glob_program);
  int max = program->count;
  int i = 0;
  while (i < max) {
    Node* node = program->nodes[i];
    switch (node->type) {
      case TNODE_ITEM:
        NERO_parse_item(node->node.item);
        break;
      default:
        printf("Unexpected Node\n");
        break;
    }
    i++;
  }
  return glob_program;
}
