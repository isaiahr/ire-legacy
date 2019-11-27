#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"core/common.h"
#include"core/error.h"
#include"lexer.h"
#include"parser.h"
#include"parseutils.h"
#include"parser_stmt.h"
#include"parser_type.h"


/***
 * 
 * Parsing is done by recursive descent using the syntax tree shown before each func
 * 
 */

// body = {[statement], term}
Lextoken* parse_body(Lextoken* p, Token* body, State* state){
    int ind = 0;
    Token* child = allocate_child_token(body, p->line);
    body->type = T_BODY;
    while(1){
        if(match(p, LEOF) || p == NULL){
            return NULL;
        }
        Lextoken* o = p;
        child->line = p->line;
        p = parse_statement(p, child, state);
        if(p != NULL){
            if(match(p, TERM)){
                // good
                child = allocate_child_token(body, p->line);
                p = next(p);
            }
            else if(match(p, RIGHT_CRPAREN)){
                // stmt without term allowed iff its last stmt in block.
                destroy_youngest(body);
                return p;
            }
            else{
                add_error(state, SYNTAXERROR, p->line, "failed to parse statement");
                child = allocate_child_token(body, p->line);
                // statement with no term, trailing lextokens after stmt, etc
                while((!match(o, TERM)) && (!match(o, LEOF))){
                    o = next(o);
                }
                p = o;
                continue;
            }
            
        }
        else if(match(o, TERM)){
            p = next(o); // empty stmt
        }
        else if(match(o, RIGHT_CRPAREN)){
            destroy_youngest(body);
            return o; // end of statements
        } else {
            add_error(state, SYNTAXERROR, o->line, "failed to parse statement");
            ind += 1;
            child = allocate_child_token(body, p->line);
        }
    }
}

 // funcdef = type, identifier, "(", [type, identifier], {"," type,  identifier}, ")", "{"
Lextoken* parse_funcdef(Lextoken* p, Token* def){
    def->type = T_FUNCDEF;
    Token* child = allocate_child_token(def, p->line);
    Token* child2 = allocate_child_token(def, p->line);
    Lextoken* l = parse_type_void(p, child);
    if(l == NULL){
        destroy_children(def);
        return NULL;
    }
    if(!match(l, IDENTIFIER)){
        destroy_children(def);
        return NULL;
    }
    def->str = malloc(strlen(l->str)+1);
    memcpy(def->str, l->str, strlen(l->str)+1);
    l = next(l);
    if((!match(l, LEFT_PAREN)) || (l == NULL)){
        destroy_children(def);
        return NULL;
    }
    l = next(l);
    child2->type = T_VARPARAM;
    Token* grandchild = allocate_child_token(child2, p->line);
    Lextoken* ty1 = parse_type(l, (grandchild));
    Lextoken* ident1 = match(ty1, IDENTIFIER) ? next(ty1) : NULL;
    
    if(ident1 != NULL){
        
        // identifier is ty1 
        child2->str = malloc(strlen(ty1->str)+1);
        memcpy(child2->str, ty1->str, strlen(ty1->str)+1);
        l = ident1;
        while(1){
            if(!match(l, COMMA)){
                break;
            }
            child2 = allocate_child_token(def, l->line);
            child2->type = T_VARPARAM;
            grandchild = allocate_child_token(child2, l->line);
            Lextoken* ty = parse_type(next(l), grandchild);
            Lextoken* ident = match(ty, IDENTIFIER) ? next(ty) : NULL;
            if(ident == NULL || ty == NULL){
                destroy_children(def);
                return NULL;
            }    
            child2->str = malloc(strlen(ty->str)+1);
            memcpy(child2->str, ty->str, strlen(ty->str)+1);
            l = ident;
        }
       
    }
    else{
        destroy_youngest(child2);
    }
    int jo = match(l, RIGHT_PAREN);
    jo = jo && match(next(l), LEFT_CRPAREN);
    if(jo){
        return next(next(l));
    }
    destroy_children(def);
    return NULL;
}

// function = funcdef, body, "}"
Lextoken* parse_function(Lextoken* p, Token* func, State* state){
    Token* child = allocate_child_token(func, p->line);
    Token* child2 = allocate_child_token(func, p->line);
    Lextoken* l = parse_funcdef(p, child);
    if(l == NULL){
        destroy_children(func);
        return NULL;
    }
    l = parse_body(l, child2, state);
    if(l != NULL && match(l, RIGHT_CRPAREN)){
        func->type = T_FUNCTION;
        return next(l);
    }
    destroy_children(func);
    return NULL;
}

// program = {[function | type] term }
Token* parse_program(Lextoken* p, State* state){
    Token* prog = init_token(p->line);
    prog->type = T_PROGRAM;
    Token* child = allocate_child_token(prog, p->line);
    int i = 0;
    while(1){
        if(match(p, TERM)){
            p = next(p);
            continue;
        }
        if(match(p, LEOF)){
            break;
        }
        Lextoken* l = parse_function(p, child, state);
        if(l == NULL){
            l = parse_typedef(p, child, state);
            if(l == NULL){
                add_error(state, SYNTAXERROR, p->line, "failed to parse function definition/body or type");
                destroy_token(prog);
                return NULL;
            }
        }
        p = l;
        i += 1;
        child = allocate_child_token(prog, p->line);
    }
    destroy_youngest(prog);
    if(state->verbose){
        print_tree(prog, 0);
    }
    return prog;
}
