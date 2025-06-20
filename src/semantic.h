#pragma once

#include <stdbool.h>
#include "types.h"
#include "node.h"
#include "internals.h"
#include "build/build_opt.h"

typedef struct {
    const char*                     name;
    Type                            type;
} Semantic_Variable;
TIX_DYN_LIST(Semantic_Variable, Semantic_Variable);
typedef struct {
    const char*                     name;
    list_Param*                     parameters;
    Type                            return_type;
    bool                            has_return_value;
    const char*                     mangled_name;
} Semantic_Function;
TIX_DYN_LIST(Semantic_Function, Semantic_Function);

typedef struct {
    list_Semantic_Variable*         variables;
    list_Semantic_Function*         functions;
} Semantic_State;

void Semantics_Init(Semantic_State* state);
void Semantical_Analysis(Semantic_State*, Program* program, char** source, BuildOptions* opts);
