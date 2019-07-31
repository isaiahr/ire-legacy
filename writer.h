#ifndef __WRITER_H__
#define __WRITER_H__
#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58) 

#include"datastructs.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdarg.h>
#include"semantic.h"
#include"writer.h"

void annotate(State* state, char* format, ...);
void write_header(State* state);
void write_footer(State* state);
void write_varinit(Variable* var, State* state);
void write_funcreturn(Function* func, Variable* var, State* state);
void write_funcdef(Function* func, State* state);
void write_funcend(Function* func, State* state);
void write_funcall(FunctionCall* func, State* state);
void write_varassign(Variable* to, Variable* from, State* state);
void write_byte(Variable* to, char byte, State* state);
void write_int(Variable* to, int immediate, State* state);
void write_string(Variable* to, char* str, int len, State* state);
void write_indget(Variable* arr, Variable* ind, Variable* to, State* state);
void write_indset(Variable* arr, Variable* ind, Variable* from, State* state);
void write_addeq(Variable* arr, Variable* delta, State* state);
void write_card(Variable* to, Variable* from, State* state);
void write_newarr(Variable* to, Variable* len, State* state);
void write_arith(Variable* to, Variable* left, Variable* right, int op, State* state);
void write_constructor(Variable* dest, int width, State* state);
void write_accessor(Variable* dest, Variable* src, int off, State* state);
void write_setmember(Variable* dest, Variable* src, int off, State* state);
void write_settag(Variable* var, int off, State* state);
void write_conditional(Variable* test, char* truelbl, char* falselbl, State* state);
void write_label(char* lbl, int uncond, State* state);

#endif
