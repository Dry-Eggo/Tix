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

NERO_Value* NERO_Value_from_storage(const char* name, NERO_Inst* val) {
    NERO_Value* value = NEW(NERO_Value);
    value->val.storage_value = NEW(NERO_Store);
    value->val.storage_value->label = name;
    value->val.storage_value->value = val;
    return value;
}

NERO_Value* NERO_Value_from_ari(NERO_Opcode op, NERO_Value* lhs, NERO_Inst* rhs) {
    NERO_Value* val    = NEW(NERO_Value);
    NERO_Ari*   ari    = NEW(NERO_Ari);
    ari->op            = op;
    ari->lhs           = lhs;
    ari->rhs           = rhs;
    val->val.ari_value = ari;
    return val;
}

NERO_Value* NERO_Value_from_load(const char* tmp, NERO_Inst* value) {
    NERO_Value* val = NEW(NERO_Value);
    val->val.load = NEW(NERO_Load);
    val->val.load->label = tmp;
    val->val.load->value = value;
    return val;
}

NERO_Value* NeroNull() { return NULL; }
NERO_Value* NERO_Value_from_label(const char* label) {
    NERO_Value* value  = NEW(NERO_Value);
    NERO_Label* labelv = NEW(NERO_Label);
    labelv->label = label;
    value->val.label_value = labelv;
    return value;
}
NERO_Value* NERO_Value_from_reserve(const char* name, int size) {
    NERO_Value* value  = NEW(NERO_Value);
    NERO_Res*   res    = NEW(NERO_Res);
    res->label         = name;
    res->size          = size;
    value->val.reserve = res;
    return               value;
}

NERO_Value* NERO_Value_from_call(const char* name, NERO_Inst** args, int argc) {
    NERO_Value* value = NEW(NERO_Value);
    NERO_Call*  call  = NEW(NERO_Call);
    call->name        = name;
    call->args        = args;
    call->argc        = argc;
    value->val.call   = call;
    return value;
}

NERO_Const* NERO_new_const(int i) {
    NERO_Const* cons = NEW(NERO_Const);
    cons->value = i;
    return cons;
}
