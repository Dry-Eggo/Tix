#include "semantic.h"


void Semantics_Init(Semantic_State* state) {
    state = NEW(Semantic_State);
    list_Semantic_Function_init(&state->functions);
    list_Semantic_Variable_init(&state->variables);
}


/*
 * Run Diagnostics on the Constructed Ast
 * Ensures that errors are reported before generating Nero Ir
*/
void Semantical_Analysis(Semantic_State* state, Program* program, char** source, BuildOptions* opts) {
    // TODO
}
