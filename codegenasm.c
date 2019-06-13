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
#include"irec.h"
#include"commitid.h"
#include"semantic.h"

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
    fprintf(state->fp, "\n\n\n.ident \"irec version %s build %s\"\n", VERSION_STRING, COMMIT_ID);
}

void awrite_varinit(Variable* var, State* state){
    fprintf(state->fp, "pushq $0\n");
}

void awrite_funcreturn(Function* func, Variable* var, State* state){
    func->writ_return = 1;
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", var->offset);
    fprintf(state->fp, "add $%i, %%rsp\nret\n", func->max_offset);
}

void awrite_funcend(Function* func, State* state){
    if(!func->writ_return){
        fprintf(state->fp, "movq $0, %%rax\n");
        fprintf(state->fp, "add $%i, %%rsp\nret\n", func->max_offset);
    }
}

void awrite_funcdef(Function* func, State* state){
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

void awrite_funcall(FunctionCall* func, State* state){
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

void awrite_varassign(Variable* to, Variable* from, State* state){
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


void awrite_byte(Variable* to, char byte, State* state){
    unsigned char b = byte;
    fprintf(state->fp, "movb $0x%x, %i(%%rsp)\n", b, to->offset);
}

void awrite_int(Variable* to, int immediate, State* state){
    fprintf(state->fp, "movq $%i, %i(%%rsp)\n", immediate, to->offset);
}

void awrite_indget(Variable* arr, Variable* ind, Variable* to, State* state){
    if(strcmp(arr->type->identifier, "Byte[]") == 0){
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", arr->offset);
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", ind->offset);
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq (%%r15), %%rax\n");
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    }
    else {
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", arr->offset);
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", ind->offset);
        fprintf(state->fp, "imul $8, %%rax, %%rax\n");
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq (%%r15), %%rax\n");
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    }
}

void awrite_indset(Variable* arr, Variable* ind, Variable* from, State* state){
    if(strcmp(arr->type->identifier, "Byte[]") == 0){
        fprintf(state->fp, "movq %i(%%rsp), %%r14\n", from->offset); // val
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset); // arr
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", ind->offset); // index
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq %%r14, (%%r15)\n");
    }
    else {
        fprintf(state->fp, "movq %i(%%rsp), %%r14\n", from->offset); // val
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset); // arr
        fprintf(state->fp, "movq %i(%%rsp), %%r15\n", ind->offset); // index
        fprintf(state->fp, "imul $8, %%r15, %%r15\n");
        fprintf(state->fp, "addq %%rax, %%r15\n");
        fprintf(state->fp, "movq %%r14, (%%r15)\n");
    }
}

void awrite_addeq(Variable* arr, Variable* delta, State* state){
    fprintf(state->fp, "movq %i(%%rsp), %%r15\n", delta->offset);
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset);
    if(strcmp(arr->type->identifier, "Byte[]") == 0){
        fprintf(state->fp, "call array_addb\n");
    }
    else {
        fprintf(state->fp, "call array_add\n");
    }
}

void awrite_string(Variable* to, char* str, int len, State* state){
    int sz = ((len / 128) * 128) + 128;
    // allocated space is len < n * 128 where n is minimized
    fprintf(state->fp, "movq $%i, %%rax\n", sz);
    fprintf(state->fp, "call alloc\n");
    fprintf(state->fp, "movq $%i, (%%rax)\n", len); // write length
    for(int i = 0; i < len; i += 1){
        fprintf(state->fp, "movq $0x%02X, %i(%%rax)\n", str[i], i+8);
    }
    fprintf(state->fp, "addq $8, %%rax\n");
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    
}

void awrite_card(Variable* to, Variable* from, State* state){
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", from->offset);
    fprintf(state->fp, "movq -8(%%rax), %%rax\n");
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
}


void awrite_newarr(Variable* to, Variable* len, State* state){
    if(strcmp(to->type->identifier, "Byte[]") == 0){
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", len->offset);
        fprintf(state->fp, "call alloc\n");
        fprintf(state->fp, "movq %i(%%rsp), %%rbx\n", len->offset);
        fprintf(state->fp, "movq %%rbx, (%%rax)\n");
        fprintf(state->fp, "addq $8, %%rax\n");
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    }
    else{
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", len->offset);
        fprintf(state->fp, "imul $8, %%rax, %%rax\n");
        fprintf(state->fp, "call alloc\n");
        fprintf(state->fp, "movq %i(%%rsp), %%rbx\n", len->offset);
        fprintf(state->fp, "movq %%rbx, (%%rax)\n");
        fprintf(state->fp, "addq $8, %%rax\n");
        fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
    }
}
