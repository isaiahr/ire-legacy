#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
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

void write_header(State* state){
    fprintf(state->fp, ".global _start\n.text\n"); 
}

void write_footer(State* state){
    fprintf(state->fp, "\n\n\n.data\n");
    List* l = state->variables;
    while(l != NULL){
        Variable* v = (Variable*) l->data;
        char* initial = "0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00";
        fprintf(state->fp, "%s: .byte %s\n", v->write_name, initial);
        l = l->next;
    }
    fprintf(state->fp, "\n\n\n.ident \"irec dev version\"\n");
}

//write refs are for passing params / or ret values.
void write_iref(char* ref, State* state){
    fprintf(state->fp, "movq $%s, %%rax\n", ref);
}
void write_varref(Variable* ref, State* state){
    fprintf(state->fp, "movq %s, %%rax\n", ref->write_name);
}
void write_funcreturn(State* state){
    fprintf(state->fp, "%s\n", "ret");
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

void write_vassign(Variable* a, Variable* b, State* state){
    //integer a = b
    char* awn = a->write_name;
    char* bwn = b->write_name;
    fprintf(state->fp, "movq %s, %%rcx\nmovq %%rcx, %s(,1)\n", bwn, awn);
}

void write_fassign(Variable* a, State* state){
    char* awn = a->write_name;
    fprintf(state->fp, "movq %%rax, %s(,1)\n", awn);
}

void write_iassign(Variable* a, char* b, State* state){
    //integer a = 5
    fprintf(state->fp, "movq $%s, %s(,1)\n", b, a->write_name);
}
