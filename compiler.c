#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"lexer.h"
#include"parser.h"
#include"compiler.h"
#include"writer.h"
#include"common.h"
#include"error.h"
#include"parser.h"
#include"semantic.h"


void compile_func(Function* f, State* state);
void compile_stmt(Statement* stmt, Function* f, State* state);

Token* parsefile(State* state, char* data){
    Lextoken* l = lex(data);
    Token* t = parse_program(l);
    return t;
}

Token* join(Token* first, Token* second){
    if(second == NULL){
        return first;
    }
    int orig = first->subtoken_count;
    first->subtoken_count = first->subtoken_count + second->subtoken_count;
    first->subtokens = realloc(first->subtokens, first->subtoken_count*sizeof(struct Token));
    for(int i = 0; i < second->subtoken_count; i++){
        memcpy(&first->subtokens[orig+i], &second->subtokens[i], sizeof(struct Token));
    }
    return first;
}

void compile(State* state, Token* t){
    Program* p = process_program(t);
    int log_c = 1;
    for(int i = 0; i < p->func_count; i++){
        // compute 10^n
        int tmp = 10;
        for(int j = 1; j < log_c; j++){
            tmp = tmp * tmp;
        }
        if(i > tmp){
            log_c += 1;
        }
        Function* f = &p->funcs[i];
        if(f->native){
            continue;
        }
        if(strcmp(f->name, ENTRYFUNC) == 0){
            f->write_name = "_start";
        }
        else{
            f->write_name = malloc(6+(int)((1+(log_c))));
            f->max_offset = 0;
            sprintf(f->write_name, "func_%i", i);
        }
        if(!f->native){
            compile_func(f, state);
        }
    }
}

void compile_func(Function* f, State* state){
    // make varlist = paramlist + varlist
    VarList* cur = f->params;
    VarList* prev = NULL;
    VarList* head = NULL;
    f->writ_return = 0;
    while(cur != NULL){
        VarList* new = malloc(sizeof(struct VarList));
        new->var = cur->var;
        if(prev != NULL){
            prev->next = new;
        }
        else{
            head = new;
        }
        prev = new;
        cur = cur->next;
    }
    if(head != NULL){
        prev->next = f->vars;
        f->vars = head;
    }
    write_funcdef(f, state);
    Body* b = f->body;
    while(b != NULL){
        compile_stmt(b->stmt, f, state);
        b = b->next;
    }
    write_funcend(f, state);
}

inline void compile_stmt(Statement* stmt, Function* f, State* state){
    switch(stmt->type){
        case S_ASSIGNMENT:
            ;
            Assignment* a = (Assignment*) stmt->stmt;
            write_varassign(a->to, a->from, state);
            break;
        case S_CONSTANTASSIGNMENT:
            ;
            ConstantAssignment* ca = (ConstantAssignment*) stmt->stmt;
            if(ca->type == S_CONST_BYTE){
                write_byte(ca->to, ca->byte, state);
            }
            if(ca->type == S_CONST_INT){
                write_int(ca->to, ca->lnt, state);
            }
            if(ca->type == S_CONST_STRING){
                write_string(ca->to, ca->string, strlen(ca->string), state);
            }
            break;
        case S_FUNCTIONCALL:
            ;
            FunctionCall* fc = (FunctionCall*) stmt->stmt;
            write_funcall(fc, state);
            break;
        case S_VARINIT:
            ;
            VarInit* vi = (VarInit*) stmt->stmt;
            VarList* vl = f->vars;
            while(vl->var != vi->var){
                vl->var->offset += 8;
                vl = vl->next;
            }
            vl->var->offset = 0;
            f->max_offset = f->vars->var->offset+8; // + 8 ??
            write_varinit(vi->var, state);
            break;
        case S_RETURN:
            ;
            Return* r = (Return*) stmt->stmt;
            write_funcreturn(f, r->var, state);
            break;
    }
}

