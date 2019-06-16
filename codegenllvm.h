#ifndef __CODEGENLLVM_H__
#define __CODEGENLLVM_H__

#include"datastructs.h"
#include"semantic.h"

void lwrite_header(State* state);
void lwrite_footer(State* state);
void lwrite_varinit(Variable* var, State* state);
void lwrite_funcreturn(Function* func, Variable* var, State* state);
void lwrite_funcend(Function* func, State* state);
void lwrite_funcdef(Function* func, State* state);
void lwrite_funcall(FunctionCall* func, State* state);
void lwrite_varassign(Variable* to, Variable* from, State* state);
void lwrite_byte(Variable* to, char byte, State* state);
void lwrite_int(Variable* to, int immediate, State* state);
void lwrite_indget(Variable* arr, Variable* ind, Variable* to, State* state);
void lwrite_indset(Variable* arr, Variable* ind, Variable* from, State* state);
void lwrite_addeq(Variable* arr, Variable* delta, State* state);
void lwrite_string(Variable* to, char* str, int len, State* state);
void lwrite_card(Variable* to, Variable* from, State* state);
void lwrite_newarr(Variable* to, Variable* len, State* state);
void lwrite_arith(Variable* to, Variable* left, Variable* right, int op, State* state);

#endif
