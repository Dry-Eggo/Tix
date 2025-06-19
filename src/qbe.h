#pragma once

#include "string_builder.h"
#include "ir/nero.h"
#include "build/build_opt.h"

typedef struct {
    const char*       name;
    NERO_Arg**        args;
    int               argc;
} Qbe_Function;
TIX_DYN_LIST(Qbe_Function, Qbe_Function);
typedef struct {
    list_NERO_Inst*     program;
    list_Qbe_Function*  functions;
    BuildOptions*       options;
    StringBuilder*      output;
    int                 tmp_counter;
} Qbe_State;

void qbe_compile(list_NERO_Inst* instructions, char** source, BuildOptions* opts);
const char* qbe_new_tmp(Qbe_State* qbe);
