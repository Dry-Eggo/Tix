#include "qbe.h"
#include "internals.h"

void qbe_generate_function(Qbe_State*,          NERO_Inst*);
void qbe_generate_declaration(Qbe_State* state, NERO_Inst* inst);
void qbe_store_function(Qbe_State* state,       NERO_Function*);
void qbe_generate_store(Qbe_State* state,       NERO_Inst* storage);
void qbe_generate_body(Qbe_State* state,        list_NERO_Inst* body);
void qbe_generate_reserve(Qbe_State* state,     NERO_Inst* inst);
void qbe_generate_call(Qbe_State* state,     NERO_Inst* inst);
void qbe_generate_ari(Qbe_State* state,     NERO_Inst* inst);
void qbe_generate_load(Qbe_State* state,     NERO_Inst* inst);

typedef struct {
    StringBuilder* result;
    StringBuilder* preamble;
} QBE_Res;

#define $R(name) QBE_Res* name = NEW(QBE_Res); SB_init(&name->result); SB_init(&name->preamble);

list_NERO_Inst* lfp(NERO_Inst** inst, int count) {
    list_NERO_Inst* list;
    list_NERO_Inst_init(&list);
    list->data = inst;
    list->count = count;
}

QBE_Res* qbe_generate_value(Qbe_State* state, NERO_Inst* value) {
    $R(res);
    if (value->opcode == NERO_OP_CONST) {
	SB_set(res->result, "%d", value->src->val.constant_value->value);
    } else if (value->opcode == NERO_OP_LABEL) {
	const char* tmp = qbe_new_tmp(state);
	SB_set(res->preamble, "    %s =w loadw %%%s\n", tmp, value->src->val.label_value->label);
	SB_set(res->result, "%s", tmp);
    }
    return res;
}

void qbe_generate_ari(Qbe_State* state,     NERO_Inst* inst) {
    switch (inst->opcode) {
    case NERO_OP_ADD:
	NERO_Ari* ari = inst->src->val.ari_value;
	QBE_Res*  rhs = qbe_generate_value(state, ari->rhs);
	const char* tmp = qbe_new_tmp(state);
	// qbe_generate_expr increments this counter when loading a label so decrement it so that STORE will get the correct label
	// TODO: distinguish between temporary labels and actual labels. NERO_Tmp maybe
	state->tmp_counter--;
	sbapp(state->output, "    %s =w add %%%s, %s\n", tmp,  ari->lhs->val.label_value->label, rhs->result->data); 
	break;
    default:
	break;
    }
}

void qbe_generate_load(Qbe_State* state,     NERO_Inst* inst) {
    NERO_Load* load = inst->src->val.load;
    QBE_Res* res    = qbe_generate_value(state, load->value);
    sbapp(state->output, "    %%%s =w add %s, 0\n", load->label, res->result->data);
}

void qbe_generate_declaration(Qbe_State* state, NERO_Inst* inst) {
    NERO_Declare* declare = inst->src->val.declaration_value;
    if (declare->type == NERO_DECLARE_FUNCTION) {
	NERO_Function* func = declare->value->src->val.function_value;
	//	sbapp(state->output, "export function $%s()\n", func->name);
	qbe_store_function(state, func);
    }
}
// TODO: NERO_Function and Qbe_Function have the same structure
void qbe_store_function(Qbe_State* state, NERO_Function* func) {
    Qbe_Function* fn = NEW(Qbe_Function);
    fn->name = func->name;
    fn->args = func->arguments;
    fn->argc = func->argc;
    list_Qbe_Function_add(state->functions, fn);
}

void qbe_generate_reserve(Qbe_State* state, NERO_Inst* inst) {
    NERO_Res* res = inst->src->val.reserve;
    char* class_ = "l";
    sbapp(state->output, "    %%%s =%s alloc%d %d\n", res->label, class_, res->size, res->size);
}

void qbe_generate_store(Qbe_State* state, NERO_Inst* inst) {
    NERO_Store* store = inst->src->val.storage_value;
    QBE_Res* expr = qbe_generate_value(state, store->value);
    sbapp(state->output, "    storew %s, %%%s\n", expr->result->data, store->label);
}

void qbe_generate_call(Qbe_State* state, NERO_Inst* inst) {
    NERO_Call*  call = inst->src->val.call;
    StringBuilder* out;
    SB_init(&out);
    sbapp(out, "    call $%s( w ", call->name);
    int i = 0;
    while ( i < call->argc) {
	NERO_Inst* arg  = call->args[i];
	QBE_Res*   expr = qbe_generate_value(state, arg);
	sbapp(state->output, "%s", expr->preamble->data);
	sbapp(out, "%s", expr->result->data);
	++i;
    }
    sbapp(out, ")\n");
    sbapp(state->output, "%s", out->data);
}

void qbe_generate_body(Qbe_State* state, list_NERO_Inst* body) {
    int max = body->count;
    int i = 0;
    while (i < max) {
	NERO_Inst* inst = list_NERO_Inst_get(body, i);
	switch(inst->opcode) {
	case NERO_OP_DECLARE:
	    qbe_generate_declaration(state, inst);
	    break;
	case NERO_OP_FUNCTION:
	    qbe_generate_function(state, inst);
	    break;
	case NERO_OP_RES:
	    qbe_generate_reserve(state, inst);
	    break;
	case NERO_OP_STORE:
	    qbe_generate_store(state, inst);
	    break;
	case NERO_OP_CALL:
	    qbe_generate_call(state, inst);
	    break;
	case NERO_OP_LOAD:
	    qbe_generate_load(state, inst);
	case NERO_OP_ADD:
	case NERO_OP_SUB:
	case NERO_OP_MUL:
	case NERO_OP_DIV:
	    qbe_generate_ari(state, inst);
	    break;
	}
	++i;
    }
}

void qbe_generate_function(Qbe_State* state, NERO_Inst* inst) {
    NERO_Function* func = inst->src->val.function_value;
    sbapp(state->output, "export function $%s(){\n", func->name);
    sbapp(state->output, "@entry\n");
    list_NERO_Inst* body = lfp(func->insts, func->size);
    qbe_generate_body(state, body);
    sbapp(state->output, "    ret\n");
    sbapp(state->output, "}\n");
    qbe_store_function(state, func);
}

void qbe_generate(Qbe_State* state) {
  list_NERO_Inst* program = state->program;
  int max = program->count;
  int i = 0;
  while (i < max) {
    NERO_Inst* instruction = list_NERO_Inst_get(program, i);
    switch(instruction->opcode) {
    case NERO_OP_DECLARE:
	qbe_generate_declaration(state, instruction);
	break;
    case NERO_OP_FUNCTION:
	qbe_generate_function(state, instruction);
	break;
    }
    ++i;
  }
}


void qbe_compile(list_NERO_Inst* instructions, char** source, BuildOptions* opts) {
  Qbe_State* state = NEW(Qbe_State);
  state->options = opts;
  SB_init(&state->output);
  state->program = instructions;
  state->tmp_counter = 0;
  list_Qbe_Function_init(&state->functions);

  qbe_generate(state);
  FILE* f = fopen(state->options->outputfile, "w");
  if (!f) {
      printf("Unable to open output file\n");
      exit(1);
  }
  fprintf(f, "%s", state->output->data);
  SB_free(state->output);
}

const char* qbe_new_tmp(Qbe_State* state) {
  char buf[16] = {0};
  sprintf(buf, "%%t%d", state->tmp_counter++);
  return strdup(buf);
}
