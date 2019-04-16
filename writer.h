#ifndef __WRITER_H__
#define __WRITER_H__
#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58) 
extern void annotate(State* state, char* format, ...);
extern void write_funcall(Function* func, State* state);
extern void write_funcdef(Function* func, State* state);
extern void write_funcreturn(State* state);
extern void write_asm(char* str, State* state);
extern void write_varinit(Variable* var, State* state);
extern void write_varref(Variable* var, State* state);
extern void write_header(State* state);
extern void write_footer(State* state);
extern void write_int(int immediate, State* state);
extern void write_byte(char byte, State* state);
extern void write_varassign(Variable* a, State* state);
extern void write_arradd(Variable* a, State* state);
extern void write_arrset(Variable* a, Variable* ind, State* state);
extern void write_arrind(Variable* a, State* state);
extern void write_string(char* str, int len, State* state);
#endif
