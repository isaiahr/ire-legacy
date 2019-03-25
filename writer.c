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
}

void write_footer(State* state){
    fprintf(state->fp, "\n\n\n.ident \"irec dev version\"\n");
}

void write_varinit(Variable* var, State* state){
    fprintf(state->fp, "pushq $0\n");
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
    if(a->type->id == VARTYPE_INTEGER){
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", a->offset);
    }
    else if (a->type->id == VARTYPE_BYTE){
        fprintf(state->fp, "movb %%al, %i(%%rsp)\n", a->offset);
    }
    else{
        fprintf(stderr, "Error computing types while compiling.\nExiting.\n");
        exit(-1);
    }
}

void write_varref(Variable* ref, State* state){
    if(ref->type->id == VARTYPE_INTEGER){
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", ref->offset);
    } else if(ref->type->id == VARTYPE_BYTE){
        fprintf(state->fp, "movb %i(%%rsp), %%al\n", ref->offset);
    }
    else{
        fprintf(stderr, "Error computing types while compiling.\nExiting.\n");
        exit(-1);
    }
}

void write_immediate(int immediate, State* state){
    fprintf(state->fp, "movq $%i, %%rax\n", immediate);
}

