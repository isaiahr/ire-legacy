#ifndef __CODEGENASM_H__
#define __CODEGENASM_H__

#include"core/common.h"
#include"ast/ast.h"

void awrite_header(State* state);
void awrite_footer(State* state);
void awrite_varinit(Variable* var, State* state);
void awrite_funcreturn(Function* func, Variable* var, State* state);
void awrite_funcend(Function* func, State* state);
void awrite_funcdef(Function* func, State* state);
void awrite_funcall(FunctionCall* func, State* state);
void awrite_varassign(Variable* to, Variable* from, State* state);
void awrite_byte(Variable* to, char byte, State* state);
void awrite_bool(Variable* to, int bool, State* state);
void awrite_int(Variable* to, int immediate, State* state);
void awrite_indget(Variable* arr, Variable* ind, Variable* to, State* state);
void awrite_indset(Variable* arr, Variable* ind, Variable* from, State* state);
void awrite_addeq(Variable* arr, Variable* delta, State* state);
void awrite_string(Variable* to, char* str, int len, State* state);
void awrite_card(Variable* to, Variable* from, State* state);
void awrite_newarr(Variable* to, Variable* len, State* state);
void awrite_invert(Variable* to, Variable* from, State* state);
void awrite_arith(Variable* to, Variable* left, Variable* right, int op, State* state);
void awrite_constructor(Variable* dest, int width, State* state);
void awrite_accessor(Variable* dest, Variable* src, int off, State* state);
void awrite_setmember(Variable* dest, Variable* src, int off, State* state);
void awrite_settag(Variable* var, int off, State* state);
void awrite_conditional(Variable* test, char* truelbl, char* falselbl, State* state);
void awrite_label(char* lbl, int uncond, State* state);
void awrite_unconditional(char* lbl, State* state);

#endif
