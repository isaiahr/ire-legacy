#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdarg.h>
#include"datastructs.h"
#include"writer.h"
#include"pre_s.h"

/**
CALLING CONVENTIONS.
param is passed as rax
ie
mov (param), rax
call func

in a func, param is rbx.
(aka)
func:
mov rax, rbx. 


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
    fprintf(state->fp, ".global _start\n.text\n");
    fprintf(state->fp, "%s\n", pre_s);
}

void write_footer(State* state){
    fprintf(state->fp, "\n\n\n.ident \"irec dev version\"\n");
}

void write_varinit(Variable* var, State* state){
    if(var->type->id < 0){
        // array
        fprintf(state->fp, "movq $1024, %%rax\n");
        fprintf(state->fp, "call alloc\n");
        fprintf(state->fp, "movq $0, (%%rax)\n");
        fprintf(state->fp, "pushq %%rax\n");
    }
    else{
        fprintf(state->fp, "pushq $0\n");
    }
}

void write_funcreturn(State* state){
    fprintf(state->fp, "add $%i, %%rsp\nret\n", state->currentfunc->max_offset);
}

void write_funcdef(Function* func, State* state){
    fprintf(state->fp, "%s:\nmovq %%rax, %%rbx\n", func->write_name);
}

void write_asm(char* str, State* state){
    fprintf(state->fp, "%s\n", str);
}

void write_funcall(Function* func, State* state){
    fprintf(state->fp, "push %%rbx\ncall %s\npop %%rbx\n", func->write_name);
}

void write_varassign(Variable* a, State* state){
    // a = b
    if(a->type->id != VARTYPE_BYTE){
        fprintf(state->fp, "movq %%%s, %i(%%rsp)\n", state->treg, a->offset);
    }
    else if (a->type->id == VARTYPE_BYTE){
        fprintf(state->fp, "movb %%%s, %i(%%rsp)\n", state->tregm, a->offset);
    }
    else{
        fprintf(stderr, "Error computing types while compiling.\nExiting.\n");
        exit(-1);
    }
}

void write_varref(Variable* ref, State* state){
    if(ref->type->id != VARTYPE_BYTE){
        fprintf(state->fp, "movq %i(%%rsp), %%%s\n", ref->offset, state->treg);
    } else if(ref->type->id == VARTYPE_BYTE){
        fprintf(state->fp, "movb %i(%%rsp), %%%s\n", ref->offset, state->tregm);
    }
    else{
        fprintf(stderr, "Error computing types while compiling.\nExiting.\n");
        exit(-1);
    }
}

void write_byte(char byte, State* state){
    unsigned char b = byte;
    fprintf(state->fp, "movb $0x%x, %%%s\n", b, state->tregm);
}

void write_int(int immediate, State* state){
    fprintf(state->fp, "movq $%i, %%%s\n", immediate, state->treg);
}

void write_arrset(Variable* arr, Variable* ind, State* state){
    if(arr->type->id == -VARTYPE_BYTE){
        fprintf(state->fp, "movq %%rax, %%r14\n"); // val
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset); // arr
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", ind->offset); // index
        fprintf(state->fp, "addq $8, %%rax\n");
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq %%r14, (%%r15)\n");
    }else{
        fprintf(state->fp, "movq %%rax, %%r14\n"); // val
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset); // arr
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", ind->offset); // index
        fprintf(state->fp, "addq $8, %%rax\n");
        fprintf(state->fp, "imul $8, %%r15, %%r15\n");
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq %%r14, (%%r15)\n");
    }
}

void write_arradd(Variable* arr, State* state){
    fprintf(state->fp, "movq %%rax, %%r15\n");
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset);
    if(arr->type->id == -VARTYPE_BYTE){
        fprintf(state->fp, "call array_addb\n");
    }else{
        fprintf(state->fp, "call array_add\n");
    }
}

void write_arrind(Variable* arr, State* state){
    if(arr->type->id == -VARTYPE_BYTE){
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", arr->offset);
        fprintf(state->fp, "addq $8, %%r15\n");
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq (%%r15), %%rax\n");
    }
    else{
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", arr->offset);
        fprintf(state->fp, "addq $8, %%r15\n");
        fprintf(state->fp, "imul $8, %%rax, %%rax\n");
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq (%%r15), %%rax\n");
    }
}

