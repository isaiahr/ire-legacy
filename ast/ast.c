#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdarg.h>

#include"parser/parser.h"
#include"ast.h"
#include"ast_print.h"
#include"ast_stmt.h"
#include"ast_manip.h"
#include"ast_types.h"
#include"core/error.h"
#include"core/common.h"
#include"parser/parseutils.h"

/**
 * ast.c -- converts the parse tree produced by the parser to the
 * abstract syntax tree defined in ast.h. this part is where semantic analysis
 * of program validity is done (type check, undefined func check, etc)
 * 
 * 
 */

void compile_function(Token* xd, Function* func, Program* prog, State* state);
void process_function(Token* xd, Function* func, Program* prog, State* state);
void process_type(Token* t, Type* y, Program* prog, State* state);
void compile_type(Token* t, Type* y, Program* prog, State* state);


Program* process_program(Token* t, State* state){
    Program* po = malloc(sizeof(struct Program));
    int num_nativefuncs = 1;
    int num_nativetypes = 3;
    po->func_count = num_nativefuncs;
    po->type_count = num_nativetypes; // (num_nativetypes)
    for(int jk = 0; jk < subtoken_count(t); jk++){
        if(subtoken(t, jk)->type == T_TYPEDEF){
            po->type_count += 1;
        }
        else if(subtoken(t, jk)->type == T_FUNCTION){
            po->func_count += 1;
        }
        else{
            exit(55); // impossible probably (tm)
        }
    }
    po->funcs = malloc(po->func_count*sizeof(struct Function));
    po->types = malloc(sizeof(struct TypeList));
    Type* Int = malloc(sizeof(struct Type));
    Type* Byte = malloc(sizeof(struct Type));
    Type* Boolean = malloc(sizeof(struct Type));
    Type* Void = malloc(sizeof(struct Type));
    Int->width = 64;
    Int->identifier = "Int";
    Int->llvm = "i64";
    Int->ts = NULL;
    Byte->width = 8;
    Byte->identifier = "Byte";
    Byte->llvm = "i8";
    Byte->ts = NULL;
    Boolean->width = 8;
    Boolean->identifier = "Boolean";
    Boolean->llvm = "i8";
    Boolean->ts = NULL;
    Void->width = 0;
    Void->identifier = "void";
    Void->llvm = "void";
    Void->ts = NULL;
    po->types->type = Int;
    po->types->next = malloc(sizeof(struct TypeList));
    po->types->next->type = Byte;
    po->types->next->next = malloc(sizeof(struct TypeList));
    po->types->next->next->type = Boolean;
    po->types->next->next->next = malloc(sizeof(struct TypeList));
    po->types->next->next->next->type = Void;
    po->types->next->next->next->next = NULL;
    po->funcs[0].name = "syscall";
    po->funcs[0].write_name = "syscall";
    po->funcs[0].retval = Int; // i64
    po->funcs[0].params = NULL;
    po->funcs[0].vars = NULL;
    po->funcs[0].body = NULL;
    po->funcs[0].param_count = 0;
    po->funcs[0].native = 1;
    
    // proc types first
    int c12 = 0;
    // latest type, curt links in new types.
    TypeList* curt = po->types->next->next->next;
    
    while(c12 < subtoken_count(t)){
        while(subtoken(t, c12)->type != T_TYPEDEF){
            c12 += 1;
            if(c12 >= subtoken_count(t)){
                break;
            }
        }
        // double break;
        if(c12 >= subtoken_count(t))
            break;
        Type* proct = malloc(sizeof(struct Type));
        process_type(subtoken(t, c12), proct, po, state);
        TypeList* new = malloc(sizeof(TypeList));
        new->next = NULL;
        new->type = proct;
        curt->next = new;
        c12 += 1;
        curt = new;
    }   
    c12 = 0;
    // has to be first new type ??
    curt = po->types->next->next->next->next;
    while(c12 < subtoken_count(t)){
        while(subtoken(t, c12)->type != T_TYPEDEF){
            c12 += 1;
            if(c12 >= subtoken_count(t)){
                break;
            }
        }
        // double break;
        if(c12 >= subtoken_count(t))
            break;
        compile_type(subtoken(t, c12), curt->type, po, state);
        char* rp123 = duptypes(curt->type);
        if(rp123){
            char* msg = format("two members named %s in type %s", rp123, curt->type->identifier);
            add_error(state, DUPMEMBERTYPE, subtoken(t, c12)->line, msg);
        }
        curt = curt->next;
        c12 += 1;
    }
    int c0 = 0;
    for(int i = num_nativefuncs; i < po->func_count; i++){
        po->funcs[i].native = 0;
        while(subtoken(t, c0)->type != T_FUNCTION){
            c0 += 1;
        }
        process_function(subtoken(t, c0), &po->funcs[i], po, state);
        c0 += 1;
    }
    
    
    for(int i = 0; i < po->func_count; i++){
        Function* f = &po->funcs[i];
        if(f->native){
            continue;
        }
        if(strcmp(f->name, ENTRYFUNC) == 0){
            f->write_name = "_start";
        }
        else{
            f->write_name = malloc(6+20);//20 digits = max of int
            f->max_offset = 0;
            sprintf(f->write_name, "func_%i", i);
        }
    }
    // seperate proccessing function header from body to enable calling funcs declared after.
    int funccount = 0;
    for(int i = 0; i < subtoken_count(t); i++){
        if(subtoken(t, i)->type == T_TYPEDEF){
            continue;
        }
        for(int j = 0; j < i; j++){
            if(subtoken(t, j)->type == T_TYPEDEF){
                continue;
            }
            if(j == i){
                continue;
            }
            if(strcmp(subtoken(subtoken(t, i), 0)->str, subtoken(subtoken(t, j), 0)->str) == 0){
                char* msg = format("function %s redefined", subtoken(subtoken(t, j), 0)->str);
                add_error(state, DUPDEFFUNC, subtoken(subtoken(t, i-num_nativefuncs), 0)->line, msg);
            }
        }
        compile_function(subtoken(t, i), &po->funcs[num_nativefuncs+funccount], po, state);
        funccount += 1;
    }
    if(state->verbose){
        print_prog(po);
    }
    return po;
}

void process_type(Token* t, Type* y, Program* prog, State* state){
    y->identifier = clone(t->str);
    y->llvm = "i64";
    y->width = 64;
}

void compile_type(Token* t, Type* y, Program* prog, State* state){
    // make sure type not redefined
    TypeList* cur = prog->types;
    while(cur != NULL){
        Type* cmp = cur->type;
        if((cmp != y) && strcmp(cmp->identifier, t->str) == 0){
            char* msg = format("type %s redefined", t->str);
            add_error(state, DUPDEFTYPE, subtoken(t, 0)->line, msg);
        }
        cur = cur->next;
    }
    y->ts = malloc(sizeof(struct TypeStructure));
    y->ts->next = NULL;
    y->ts->sub = NULL;
    write_structure(y->ts, subtoken(t, 0), prog, state);
    y->internal_width = bytes(y->ts);
    y->llvm = malloc(32);
    snprintf(y->llvm, 31, "i%i*", y->internal_width);
    y->llvm[31] = 0;
    y->width = 64;
}

void process_function(Token* xd, Function* func, Program* prog, State* state){
    Token def = *subtoken(xd, 0);
    func->name = clone(def.str);
    func->retval = proc_type(subtoken(&def, 0)->str, prog);
    if(func->retval == NULL){
        add_error(state, UNDEFTYPE, subtoken(&def, 0)->line, subtoken(&def, 0)->str);
    }
    func->params = NULL;
    if(subtoken_count(&def) == 2 && subtoken_count(subtoken(&def, 1)) == 0){
        func->param_count = 0; // for empty varparam
    } else {
        func->param_count = (subtoken_count(&def) - 1);
    }
    func->vars = NULL;
    func->body = NULL;
    for(int i = 1; i < subtoken_count(&def); i++){
        if(subtoken(&def, i)->str == NULL){
            continue; // empty varparam
        }
        char* ident = clone(subtoken(&def, i)->str);
        Type* t = proc_type(subtoken(subtoken(&def, i), 0)->str, prog);
        Variable* var = malloc(sizeof(struct Variable));
        var->inited = 0;
        var->type = t;
        var->meta = NULL;
        var->identifier = ident;
        func->params = add_varlist(func->params, var);
    }
}

void compile_function(Token* t, Function* f, Program* prog, State* state){
    for(int i = 0; i < subtoken_count(subtoken(t, 1)); i++){
        process_stmt(subtoken(subtoken(t, 1), i), f, NULL, prog, state);
    }
    if(subtoken(subtoken(t, 1), subtoken_count(subtoken(t, 1)) - 1)->type != T_RETURN){
        // allow void
        if(f->retval == proc_type("void", prog)){
            return;
        }
        add_error(state, INCOMPATTYPE, t->line, format("returning void from function %s of type %s", f->name, f->retval->identifier));
    }
}
