#pragma once

typedef struct {
    const char*      name;
    NERO_Type        type;
} Variable;
TIX_DYN_LIST(Variable, Variable);

typedef struct {
    const char*      name;
    int              argc;
} Function;

typedef struct {
    list_NERO_Var* variables;
} NERO_State;
