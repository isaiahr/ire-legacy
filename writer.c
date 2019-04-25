#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include"datastructs.h"
#include"codegenasm.h"
#include"codegenllvm.h"

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
    va_list args;
    va_start(args, format);
    vfprintf(state->fp, format, args);
    va_end(args);
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

void write_funcreturn(State* state){
    if(state->llvm){
        lwrite_funcreturn(state);
    }
    else{
        awrite_funcreturn(state);
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

void write_asm(char* str, State* state){
    if(state->llvm){
        lwrite_asm(str, state);
    }
    else{
        awrite_asm(str, state);
    }
}

void write_funcall(Function* func, State* state){
    if(state->llvm){
        lwrite_funcall(func, state);
    }
    else{
        awrite_funcall(func, state);
    }
    
}

void write_varassign(Variable* a, State* state){
    if(state->llvm){
        lwrite_varassign(a, state);
    }
    else{
        awrite_varassign(a, state);
    }
}

void write_varref(Variable* ref, State* state){
    if(state->llvm){
        lwrite_varref(ref, state);
    }
    else{
        awrite_varref(ref, state);
    }
}

void write_byte(char byte, State* state){
    if(state->llvm){
        lwrite_byte(byte, state);
    }
    else{
        awrite_byte(byte, state);
    }
}

void write_int(int immediate, State* state){
    if(state->llvm){
        lwrite_int(immediate, state);
    }
    else{
        awrite_int(immediate, state);
    }
}

void write_arrset(Variable* arr, Variable* ind, State* state){
    if(state->llvm){
        lwrite_arrset(arr, ind, state);
    } 
    else{
        awrite_arrset(arr, ind, state);
    }
}

void write_arradd(Variable* arr, State* state){
    if(state->llvm){
        lwrite_arradd(arr, state);
    }
    else{
        awrite_arradd(arr, state);
    }
}

void write_arrind(Variable* arr, State* state){
    if(state->llvm){
        lwrite_arrind(arr, state);
    }
    else{
        awrite_arrind(arr, state);
    }
}

void write_string(char* str, int len, State* state){
    if(state->llvm){
        lwrite_string(str, len, state);
    }
    else{
        awrite_string(str, len, state);
    }
}
