#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"datastructs.h"
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



void awrite_header(State* state){
    fprintf(state->fp, ".global _start\n.text\n");
    fprintf(state->fp, "%s\n", pre_s);
}

void awrite_footer(State* state){
    fprintf(state->fp, "\n\n\n.ident \"irec dev version\"\n");
}

void awrite_varinit(Variable* var, State* state){
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

void awrite_funcreturn(State* state){
    fprintf(state->fp, "add $%i, %%rsp\nret\n", state->currentfunc->max_offset);
}

void awrite_funcdef(Function* func, State* state){
    fprintf(state->fp, "%s:\nmovq %%rax, %%rbx\n", func->write_name);
}

void awrite_asm(char* str, State* state){
    fprintf(state->fp, "%s\n", str);
}

void awrite_funcall(Function* func, State* state){
    fprintf(state->fp, "push %%rbx\ncall %s\npop %%rbx\n", func->write_name);
}

void awrite_varassign(Variable* a, State* state){
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

void awrite_varref(Variable* ref, State* state){
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

void awrite_byte(char byte, State* state){
    unsigned char b = byte;
    fprintf(state->fp, "movb $0x%x, %%%s\n", b, state->tregm);
}

void awrite_int(int immediate, State* state){
    fprintf(state->fp, "movq $%i, %%%s\n", immediate, state->treg);
}

void awrite_arrset(Variable* arr, Variable* ind, State* state){
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

void awrite_arradd(Variable* arr, State* state){
    fprintf(state->fp, "movq %%rax, %%r15\n");
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset);
    if(arr->type->id == -VARTYPE_BYTE){
        fprintf(state->fp, "call array_addb\n");
    }else{
        fprintf(state->fp, "call array_add\n");
    }
}

void awrite_arrind(Variable* arr, State* state){
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

void awrite_string(char* str, int len, State* state){
    int sz = ((len / 128) * 128) + 128;
    // allocated space is len < n * 128 where n is minimized
    fprintf(state->fp, "movq $%i, %%rax\n", sz);
    fprintf(state->fp, "call alloc\n");
    fprintf(state->fp, "movq $%i, (%%rax)\n", len); // write length
    for(int i = 0; i < len; i += 1){
        fprintf(state->fp, "movq $0x%02X, %i(%%rax)\n", str[i], i+8);
    }
    
}
