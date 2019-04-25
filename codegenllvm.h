#ifndef __CODEGENLLVM_H__
#define __CODEGENLLVM_H__
extern void lwrite_funcall(Function* func, State* state);
extern void lwrite_funcdef(Function* func, State* state);
extern void lwrite_funcreturn(State* state);
extern void lwrite_asm(char* str, State* state);
extern void lwrite_varinit(Variable* var, State* state);
extern void lwrite_varref(Variable* var, State* state);
extern void lwrite_header(State* state);
extern void lwrite_footer(State* state);
extern void lwrite_int(int immediate, State* state);
extern void lwrite_byte(char byte, State* state);
extern void lwrite_varassign(Variable* a, State* state);
extern void lwrite_arradd(Variable* a, State* state);
extern void lwrite_arrset(Variable* a, Variable* ind, State* state);
extern void lwrite_arrind(Variable* a, State* state);
extern void lwrite_string(char* str, int len, State* state);
#endif
