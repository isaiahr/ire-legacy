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
#include"pre_s.h"

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

<<<<<<< HEAD
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
=======

void write_varinit(Variable* var, State* state){
    fprintf(state->fp, "pushq $0\n");
}

void write_funcreturn(Function* func, Variable* var, State* state){
    func->writ_return = 1;
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", var->offset);
    fprintf(state->fp, "add $%i, %%rsp\nret\n", func->max_offset);
}

void write_funcend(Function* func, State* state){
    if(!func->writ_return){
        fprintf(state->fp, "movq $0, %%rax\n");
        fprintf(state->fp, "add $%i, %%rsp\nret\n", func->max_offset);
    }
}

void write_funcdef(Function* func, State* state){
    fprintf(state->fp, "%s:\n", func->write_name);
    char *regs[] = {"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "r11", "r12", "r13", "r14", "r15"};
    VarList* cur = func->params;
    int i = 0;
    while(cur != NULL){
        fprintf(state->fp, "pushq $0\n");
        fprintf(state->fp, "movq %%%s, %i(%%rsp)\n", regs[i], 0);
        cur->var->offset = func->param_count*8 - (i+1)*8;
        cur = cur->next;
        i += 1;
    }
}

void write_funcall(FunctionCall* func, State* state){
    char *regs[] = {"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "r11", "r12", "r13", "r14", "r15"};
    VarList* cur = func->vars;
    int i = 0;
    while(cur != NULL){
        fprintf(state->fp, "movq %i(%%rsp), %%%s\n", cur->var->offset, regs[i]);
        cur = cur->next;
        i += 1;
    }
    fprintf(state->fp, "call %s\n", func->func->write_name);
    if(func->to != NULL){
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", func->to->offset);
    }
}

void write_varassign(Variable* to, Variable* from, State* state){
    // to = from
    if(to->type->width == 8){
        fprintf(state->fp, "movb %i(%%rsp), %%al\n", from->offset);
        fprintf(state->fp, "movb %%al, %i(%%rsp)\n", to->offset);
    }
    else{
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", from->offset);
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    }
}


void write_byte(Variable* to, char byte, State* state){
    unsigned char b = byte;
    fprintf(state->fp, "movb $0x%x, %i(%%rsp)\n", b, to->offset);
}

void write_int(Variable* to, int immediate, State* state){
    fprintf(state->fp, "movq $%i, %i(%%rsp)\n", immediate, to->offset);
}



void write_string(Variable* to, char* str, int len, State* state){
    int sz = ((len / 128) * 128) + 128;
    // allocated space is len < n * 128 where n is minimized
    fprintf(state->fp, "movq $%i, %%rax\n", sz);
    fprintf(state->fp, "call alloc\n");
    fprintf(state->fp, "movq $%i, (%%rax)\n", len); // write length
    for(int i = 0; i < len; i += 1){
        fprintf(state->fp, "movq $0x%02X, %i(%%rax)\n", str[i], i+8);
    }
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    
>>>>>>> compilation rewrite
}
