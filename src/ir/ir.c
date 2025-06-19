#include "nero.h"
#include "../internals.h"



// TODO: match to tix types
NERO_Value* NeroInt(NERO_Const* i) {
  NERO_Value* val = NEW(NERO_Value);
  val->type = NERO_Double_word;
  val->val.constant_value = i;
  return val;
}

NERO_Inst* NERO_make_inst(NERO_Opcode op, NERO_Value* src, NERO_Value* dst) {
    NERO_Inst* inst = NEW(NERO_Inst);
    inst->opcode = op; inst->src = src; inst->dst = dst;
    return inst;
}

NERO_Value* NERO_Value_from_fn(NERO_Function* fn) {
    NERO_Value* value = NEW(NERO_Value);
    value->val.function_value = fn;
    return value;
}

NERO_Value* NERO_Value_from_declaration(NERO_Declare* d) {
    NERO_Value* value = NEW(NERO_Value);
    value->val.declaration_value = d;
    return value;
}

NERO_Value* NERO_Value_from_storage(NERO_Store* s) {
    NERO_Value* value = NEW(NERO_Value);
    value->val.storage_value = s;
    return value;
}

NERO_Const* NERO_new_const(int i) {
    NERO_Const* cons = NEW(NERO_Const);
    cons->value = i;
    return cons;
}
