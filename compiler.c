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
    Lextoken* l = lex(data, state);
    mark(state);
    Token* t = parse_program(l, state);
    mark(state);
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
    Program* p = process_program(t, state);
    mark(state);
    // at this point, any error stopping the program from producing code and running normally
    // should be considered a bug.
    for(int i = 0; i < p->func_count; i++){
        Function* f = &p->funcs[i];
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
        case S_INDEX:
            ;
            Index* i = (Index*) stmt->stmt;
            write_indget(i->arr, i->ind, i->to, state);
            break;
        case S_INDEXEQUALS:
            ;
            IndexEquals* ie = (IndexEquals*) stmt->stmt;
            write_indset(ie->arr, ie->ind, ie->eq, state);
            break;
        case S_ADDEQUALS:
            ;
            AddEquals* ae = (AddEquals*) stmt->stmt;
            write_addeq(ae->var, ae->delta, state);
            break;
        case S_CARDINALITY:
            ;
            Cardinality* card = (Cardinality*) stmt->stmt;
            write_card(card->to, card->from, state);
            break;
        case S_NEWARRAY:
            ;
            NewArray* ne = (NewArray*) stmt->stmt;
            write_newarr(ne->to, ne->size, state);
            break;
        case S_RETURN:
            ;
            Return* r = (Return*) stmt->stmt;
            write_funcreturn(f, r->var, state);
            break;
    }
}

