#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"parser/lexer.h"
#include"parser/parser.h"
#include"common/compiler.h"
#include"codegen/writer.h"
#include"common/common.h"
#include"common/error.h"
#include"ast/ast.h"


void compile_func(Function* f, State* state);
void compile_stmt(Statement* stmt, Function* f, Scope* scope, State* state);
void increment_vars(Scope* scope, Function* func, State* state, int inc);

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
    annotate(state, "Function defn for %s", f->name);
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
        cur->var->inited = 1;
        cur = cur->next;
    }
    if(head != NULL){
        prev->next = f->vars;
        f->vars = head;
    }
    write_funcdef(f, state);
    Body* b = f->body;
    while(b != NULL){
        compile_stmt(b->stmt, f, NULL, state);
        b = b->next;
    }
    f->max_offset = f->vars->var->offset+8;
    write_funcend(f, state);
}

inline void compile_stmt(Statement* stmt, Function* f, Scope* scope, State* state){
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
            annotate(state, "call func %s", fc->func->name);
            write_funcall(fc, state);
            break;
        case S_VARINIT:
            ;
            VarInit* vi = (VarInit*) stmt->stmt;
            if(vi->var->identifier != NULL){
                annotate(state, "init var %s", vi->var->identifier);
            }
            increment_vars(scope, f, state, 8);
            vi->var->inited = 1;
            vi->var->offset = 0;
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
        case S_ARITHMETIC:
            ;
            Arithmetic* arith = (Arithmetic*) stmt->stmt;
            write_arith(arith->to, arith->left, arith->right, arith->operation, state);
            break;
        case S_CONSTRUCTOR:
            ;
            Constructor* cons = (Constructor*) stmt->stmt;
            write_constructor(cons->to, cons->type->internal_width, state);
            break;
        case S_ACCESSOR:
            ;
            Accessor* acce = (Accessor*) stmt->stmt;
            write_accessor(acce->to, acce->src, acce->offsetptr, state);
            break;
        case S_SETMEMBER:
            ;
            Setmember* setm = (Setmember*) stmt->stmt;
            write_setmember(setm->dest, setm->from, setm->offsetptr, state);
            break;
        case S_SETTAG:
            ;
            SetTag* sett = (SetTag*) stmt->stmt;
            write_settag(sett->dest, sett->offsetptr, state);
            break;
        case S_RETURN:
            ;
            Return* r = (Return*) stmt->stmt;
            f->max_offset = f->vars->var->offset+8;
            write_funcreturn(f, r->var, state);
            break;
        case S_IF:
            ;
            IfStmt* ifs = (IfStmt*) stmt->stmt;
            // write cond, then label, then label, and decrement vars.
            write_conditional(ifs->test, ifs->truelbl, ifs->endlbl, state);
            write_label(ifs->truelbl, 0, state);
            Body* body = ifs->scope->body;
            while(body != NULL){
                compile_stmt(body->stmt, f, ifs->scope, state);
                body = body->next;
            }
            // now sub from other vars.
            int numdec = 0;
            if(ifs->scope->vars != NULL && ifs->scope->vars->var != NULL){
                numdec = -ifs->scope->vars->var->offset - 8;
                if(numdec != 0){
                    increment_vars(ifs->scope->parent, f, state, numdec);
                }
            }
            write_label(ifs->endlbl, -numdec, state);
            break;
    }
}

void increment_vars(Scope* scope, Function* func, State* state, int increment){
    VarList* vl;
    if(scope != NULL){
        vl = scope->vars;
    }
    else{
        vl = func->vars;
    }

    while(vl != NULL){
        if(vl->var->inited){
            vl->var->offset += increment;
        }
        else{
            break;
        }
        vl = vl->next;
    }
    if(scope != NULL){
        increment_vars(scope->parent, func, state, increment);
    }
    return;
}
