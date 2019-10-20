#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"core/common.h"
#include"build/pre_s.h"
#include"irec.h"
#include"build/commitid.h"
#include"ast/ast.h"

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
    if(var != NULL){
        fprintf(state->fp, "movq %i(%%rsp), %%rax\n", var->offset);
    }
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
    if(func->to != NULL && strcmp(func->to->type->identifier, "void") != 0){
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
void awrite_bool(Variable* to, int bool, State* state){
    if(bool){
        fprintf(state->fp, "movb $1, %i(%%rsp)\n", to->offset);
    }
    else{
        fprintf(state->fp, "movb $0, %i(%%rsp)\n", to->offset);
    }
}

void awrite_int(Variable* to, int immediate, State* state){
    fprintf(state->fp, "movq $%i, %i(%%rsp)\n", immediate, to->offset);
}

void awrite_indget(Variable* arr, Variable* ind, Variable* to, State* state){
    fprintf(state->fp, "movq %i(%%rsp), %%r15\n", arr->offset);
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", ind->offset);
    switch(arr->type->array_subtype->width){
        case 64:
            fprintf(state->fp, "imul $8, %%rax, %%rax\n");
            break;
        case 32:
            fprintf(state->fp, "imul $4, %%rax, %%rax\n");
            break;
        case 16:
            fprintf(state->fp, "imul $2, %%rax, %%rax\n");
            break;
        case 8:
            break;
    }
    fprintf(state->fp, "addq %%rax, %%r15\n");
    switch(arr->type->array_subtype->width){
        case 64:
            fprintf(state->fp, "movq (%%r15), %%rax\n");
            break;
        case 32:
            fprintf(state->fp, "movq (%%r15), %%eax\n");
            break;
        case 16:
            fprintf(state->fp, "movq (%%r15), %%ax\n");
            break;
        case 8:
            fprintf(state->fp, "movq (%%r15), %%al\n");
            break;
    }
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
}

void awrite_indset(Variable* arr, Variable* ind, Variable* from, State* state){
    fprintf(state->fp, "movq %i(%%rsp), %%r14\n", from->offset); // val
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", arr->offset); // arr
    fprintf(state->fp, "movq %i(%%rsp), %%r15\n", ind->offset); // index
    switch(arr->type->array_subtype->width){
        case 64:
            fprintf(state->fp, "imul $8, %%r15, %%r15\n");
            break;
        case 32:
            fprintf(state->fp, "imul $4, %%r15, %%r15\n");
            break;
        case 16:
            fprintf(state->fp, "imul $2, %%r15, %%r15\n");
            break;
        case 8:
            break;
    }
    fprintf(state->fp, "addq %%rax, %%r15\n");
    switch(arr->type->array_subtype->width){
        case 64:
            fprintf(state->fp, "movq %%r14, (%%r15)\n");
            break;
        case 32:
            fprintf(state->fp, "movl %%r14d, (%%r15)\n");
            break;
        case 16:
            fprintf(state->fp, "movw %%r14w, (%%r15)\n");
        case 8:
            fprintf(state->fp, "movb %%r14b, (%%r15)\n");
            break;
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
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", len->offset);
    switch(to->type->array_subtype->width){
    case 64:
        fprintf(state->fp, "imul $8, %%rax, %%rax\n");
        break;
    case 32:
        fprintf(state->fp, "imul $4, %%rax, %%rax\n");
        break;
    case 16:
        fprintf(state->fp, "imul $2, %%rax, %%rax\n");
        break;
    case 8:
        break;
    }
    fprintf(state->fp, "addq $8, %%rax\n");
    fprintf(state->fp, "call alloc\n");
    fprintf(state->fp, "movq %i(%%rsp), %%rbx\n", len->offset);
    fprintf(state->fp, "movq %%rbx, (%%rax)\n");
    fprintf(state->fp, "addq $8, %%rax\n");
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
}

void awrite_invert(Variable* to, Variable* from, State* state){
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", from->offset);
    fprintf(state->fp, "xorq $1, %%rax\n");
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
}

void awrite_arith(Variable* to, Variable* left, Variable* right, int op, State* state){
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", left->offset);
    fprintf(state->fp, "movq %i(%%rsp), %%rbx\n", right->offset);
    if(op == FSLASH || op == PERCENT){
        fprintf(state->fp, "cqto\n");
        fprintf(state->fp, "idivq %%rbx\n");
        if(op == FSLASH){
            fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
        }
        else{
            fprintf(state->fp, "movq %%rdx, %i(%%rsp)\n", to->offset);
        }
        return;
    }
    switch(op){
        case PLUS:
            fprintf(state->fp, "addq %%rax, %%rbx\n");
            break;
        case DOUBLEEQUALS:
            //TODO reduce # of ops here ? 
            fprintf(state->fp, "xor %%r10, %%r10\n");
            fprintf(state->fp, "cmpq %%rax, %%rbx\n"); // rbx - rax
            fprintf(state->fp, "movq $1, %%rcx\n");
            fprintf(state->fp, "cmovzq %%rcx, %%r10\n");
            fprintf(state->fp, "movq %%r10, %i(%%rsp)\n", to->offset);
            return;
        case LESS:
            fprintf(state->fp, "xor %%r10, %%r10\n");
            fprintf(state->fp, "cmpq %%rbx, %%rax\n"); // left - right (l < r) 
            fprintf(state->fp, "movq $1, %%rcx\n");
            fprintf(state->fp, "cmovsq %%rcx, %%r10\n");
            fprintf(state->fp, "movq %%r10, %i(%%rsp)\n", to->offset);
            return;
        case GREATER:
            fprintf(state->fp, "xor %%r10, %%r10\n");
            fprintf(state->fp, "cmpq %%rax, %%rbx\n"); // left - right (l < r) 
            fprintf(state->fp, "movq $1, %%rcx\n");
            fprintf(state->fp, "cmovsq %%rcx, %%r10\n");
            fprintf(state->fp, "movq %%r10, %i(%%rsp)\n", to->offset);
            return;
        case SUBTRACT:
            fprintf(state->fp, "subq %%rbx, %%rax\n");
            fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", to->offset);
            return;
        case MULT:
            fprintf(state->fp, "imul %%rax, %%rbx\n");
            break;
        case AMPERSAND:
            fprintf(state->fp, "and %%rax, %%rbx\n");
            break;
    }
    fprintf(state->fp, "movq %%rbx, %i(%%rsp)\n", to->offset);
}

void awrite_constructor(Variable* dest, int width, State* state){
    fprintf(state->fp, "movq $%i, %%rax\n", width/8);
    fprintf(state->fp, "call alloc\n");
    fprintf(state->fp, "movq %%rax, %i(%%rsp)\n", dest->offset);
    
}
void awrite_accessor(Variable* dest, Variable* src, int off, State* state){
    // dest = src {off}
    // NOTE: off needs to be multiple of 8.
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", src->offset);
    if(dest->type->width == 64){
        fprintf(state->fp, "movq %i(%%rax), %%rbx\n", off/8); 
        fprintf(state->fp, "movq %%rbx, %i(%%rsp)\n", dest->num);
    }
    else if(dest->type->width == 32){
        fprintf(state->fp, "movl %i(%%rax), %%ebx\n", off/8); 
        fprintf(state->fp, "movl %%ebx, %i(%%rsp)\n", dest->num);
    }
    else if(dest->type->width == 16){
        fprintf(state->fp, "movw %i(%%rax), %%bx\n", off/8);
        fprintf(state->fp, "movw %%bx, %i(%%rsp)\n", dest->num);
    }
    else if(dest->type->width == 8){
        fprintf(state->fp, "movb %i(%%rax), %%bl\n", off/8);
        fprintf(state->fp, "movb %%bx, %i(%%rsp)\n", dest->num);
    }
}
void awrite_setmember(Variable* dest, Variable* src, int off, State* state){
    // dest {off} = src
    fprintf(state->fp, "movq %i(%%rsp), %%rax\n", dest->offset);
    if(src->type->width == 64){
        fprintf(state->fp, "movq %i(%%rsp), %%rbx\n", src->offset);
        fprintf(state->fp, "movq %%rbx, %i(%%rax)\n", off/8);
    }
    else if(src->type->width == 32){
        fprintf(state->fp, "movl %i(%%rsp), %%ebx\n", src->offset);
        fprintf(state->fp, "movl %%ebx, %i(%%rax)\n", off/8);
    }
    else if(src->type->width == 16){
        fprintf(state->fp, "movw %i(%%rsp), %%bx\n", src->offset);
        fprintf(state->fp, "movw %%bx, %i(%%rax)\n", off/8);
    }
    else if(src->type->width == 8){
        fprintf(state->fp, "movb %i(%%rsp), %%bl\n", src->offset);
        fprintf(state->fp, "movb %%bl, %i(%%rax)\n", off/8);
    }
}

void awrite_settag(Variable* var, int off, State* state){
    // TODO
}

void awrite_conditional(Variable* test, char* truelbl, char* falselbl, State* state){
    fprintf(state->fp, "cmp $0, %i(%%rsp)\n", test->offset);
    fprintf(state->fp, "je %s\n", falselbl);
    // could just fallthrough instead.
    fprintf(state->fp, "jmp %s\n", truelbl);
}

void awrite_unconditional(char* lbl, State* state){
    fprintf(state->fp, "jmp %s\n", lbl);
}

// multipurpose hack
void awrite_label(char* lbl, int numdec, State* state){
    if(numdec != 0){
        fprintf(state->fp, "add $%i, %%rsp\n", numdec);
    }
    if(lbl != NULL){
        fprintf(state->fp, "%s:\n", lbl);
    }
}
