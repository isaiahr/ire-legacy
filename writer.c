#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include"datastructs.h"
#include"codegenasm.h"
#include"codegenllvm.h"
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdarg.h>
#include"semantic.h"
#include"writer.h"

/**
*
*  Wrapper for codegenasm / codegenllvm
* 
* 
* 
* 
 */

void annotate(State* state, char* format, ...){
    if(!state->annotate)
        return;
    if(state->llvm){
        fprintf(state->fp, ";");
    }
    else{
        fprintf(state->fp, "#");
    }
    va_list args;
    va_start(args, format);
    vfprintf(state->fp, format, args);
    va_end(args);
    fprintf(state->fp, "\n");
}


void write_header(State* state){
    if(state->llvm){
        lwrite_header(state);
    }
    else{
        awrite_header(state);
    }
}

void write_footer(State* state){
    if(state->llvm){
        lwrite_footer(state);
    }
    else{
        awrite_footer(state);
    }
}

void write_varinit(Variable* var, State* state){
    if(state->llvm){
        lwrite_varinit(var, state);
    }
    else{
        awrite_varinit(var, state);
    }
}

void write_funcreturn(Function* func, Variable* var, State* state){
    if(state->llvm){
        lwrite_funcreturn(func, var, state);
    }
    else{
        awrite_funcreturn(func, var, state);
    }
}

void write_funcend(Function* func, State* state){
    if(state->llvm){
        lwrite_funcend(func, state);
    }
    else {
        awrite_funcend(func, state);
    }
}

void write_funcdef(Function* func, State* state){
    if(state->llvm){
        lwrite_funcdef(func, state);
    }
    else{
        awrite_funcdef(func, state);
    }
}

void write_funcall(FunctionCall* func, State* state){
    if(state->llvm){
        lwrite_funcall(func, state);
    }
    else{
        awrite_funcall(func, state);
    }
    
}

void write_varassign(Variable* to, Variable* from, State* state){
    if(state->llvm){
        lwrite_varassign(to, from, state);
    }
    else{
        awrite_varassign(to, from, state);
    }
}

void write_byte(Variable* to, char byte, State* state){
    if(state->llvm){
        lwrite_byte(to, byte, state);
    }
    else{
        awrite_byte(to, byte, state);
    }
}

void write_int(Variable* to, int immediate, State* state){
    if(state->llvm){
        lwrite_int(to, immediate, state);
    }
    else{
        awrite_int(to, immediate, state);
    }
}

void write_indget(Variable* arr, Variable* ind, Variable* to, State* state){
    if(state->llvm){
        lwrite_indget(arr, ind, to, state);
    } 
    else{
        awrite_indget(arr, ind, to, state);
    }
}

void write_indset(Variable* arr, Variable* ind, Variable* from, State* state){
    if(state->llvm){
        lwrite_indset(arr, ind, from, state);
    }
    else{
        awrite_indset(arr, ind, from, state);
    }
}

void write_addeq(Variable* arr, Variable* delta, State* state){
    if(state->llvm){
        lwrite_addeq(arr, delta, state);
    }
    else{
        awrite_addeq(arr, delta, state);
    }
}

void write_string(Variable* to, char* str, int len, State* state){
    if(state->llvm){
        lwrite_string(to, str, len, state);
    }
    else{
        awrite_string(to, str, len, state);
    }
}

void write_card(Variable* to, Variable* from, State* state){
    if(state->llvm){
        lwrite_card(to, from, state);
    }
    else {
        awrite_card(to, from, state);
    }
}

void write_newarr(Variable* to, Variable* size, State* state){
    if(state->llvm){
        lwrite_newarr(to, size, state);
    }
    else {
        awrite_newarr(to, size, state);
    }
}

void write_arith(Variable* to, Variable* left, Variable* right, int op, State* state){
    if(state->llvm){
        lwrite_arith(to, left, right, op, state);
    }
    else {   
        awrite_arith(to, left, right, op, state);
    }
}

void write_constructor(Variable* dest, int width, State* state){
    if(state->llvm){
        lwrite_constructor(dest, width, state);
    }
    else{
        awrite_constructor(dest, width, state);
    }
}
void write_accessor(Variable* dest, Variable* src, int off, State* state){
    if(state->llvm){
        lwrite_accessor(dest, src, off, state);
    }
    else{
        awrite_accessor(dest, src, off, state);
    }
}
void write_setmember(Variable* dest, Variable* src, int off, State* state){
    if(state->llvm){
        lwrite_setmember(dest, src, off, state);
    }
    else{
        awrite_setmember(dest, src, off, state);
    }
}
void write_settag(Variable* var, int off, State* state){
    if(state->llvm){
        lwrite_settag(var, off, state);
    }
    else{
        awrite_settag(var, off, state);
    }
}
