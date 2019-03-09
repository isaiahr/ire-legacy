#ifndef __WRITER_H__
#define __WRITER_H__
#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58) 
extern void write_funcall(char* func, State* state);
extern void write_funcdef(char* func, State* state);
extern void write_funcreturn(State* state);
extern void write_asm(char* str, State* state);
extern void write_varref(char* var, State* state);
extern void write_header(State* state);
extern void write_footer(State* state);
#endif
