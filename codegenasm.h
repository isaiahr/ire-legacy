#ifndef __CODEGENASM_H__
#define __CODEGENASM_H__

#include"datastructs.h"
#include"semantic.h"

void awrite_header(State* state);
void awrite_footer(State* state);
void awrite_varinit(Variable* var, State* state);
void awrite_funcreturn(Function* func, Variable* var, State* state);
void awrite_funcend(Function* func, State* state);
void awrite_funcdef(Function* func, State* state);
void awrite_funcall(FunctionCall* func, State* state);
void awrite_varassign(Variable* to, Variable* from, State* state);
void awrite_byte(Variable* to, char byte, State* state);
void awrite_int(Variable* to, int immediate, State* state);
void awrite_indget(Variable* arr, Variable* ind, Variable* to, State* state);
void awrite_indset(Variable* arr, Variable* ind, Variable* from, State* state);
void awrite_addeq(Variable* arr, Variable* delta, State* state);
void awrite_string(Variable* to, char* str, int len, State* state);
void awrite_card(Variable* to, Variable* from, State* state);
void awrite_newarr(Variable* to, Variable* len, State* state);

#endif
