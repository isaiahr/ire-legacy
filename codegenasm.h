#ifndef __CODEGENASM_H__
#define __CODEGENASM_H__
extern void awrite_funcall(Function* func, State* state);
extern void awrite_funcdef(Function* func, State* state);
extern void awrite_funcreturn(State* state);
extern void awrite_asm(char* str, State* state);
extern void awrite_varinit(Variable* var, State* state);
extern void awrite_varref(Variable* var, State* state);
extern void awrite_header(State* state);
extern void awrite_footer(State* state);
extern void awrite_int(int immediate, State* state);
extern void awrite_byte(char byte, State* state);
extern void awrite_varassign(Variable* a, State* state);
extern void awrite_arradd(Variable* a, State* state);
extern void awrite_arrset(Variable* a, Variable* ind, State* state);
extern void awrite_arrind(Variable* a, State* state);
extern void awrite_string(char* str, int len, State* state);
#endif
