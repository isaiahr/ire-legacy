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
    body->subtokens = init_token(p->line);
    body->type = T_BODY;
    while(1){
        if(match(p, LEOF) || p == NULL){
            return NULL;
        }
        Lextoken* o = p;
        body->subtokens[ind].line = p->line;
        p = parse_statement(p, &body->subtokens[ind], state);
        if(p != NULL){
            if(match(p, TERM)){
                // good
                ind += 1;
                body->subtokens = realloc_token(body->subtokens, (ind+1));
                body->subtoken_count = ind;
                p = next(p);
            }
            else if(match(p, RIGHT_CRPAREN)){
                // stmt without term allowed iff its last stmt in block.
                ind += 1;
                body->subtoken_count = ind;
                return p;
            }
            else{
                add_error(state, SYNTAXERROR, p->line, "failed to parse statement");
                ind += 1;
                body->subtokens = realloc_token(body->subtokens, (ind+1));
                body->subtoken_count = ind;
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
            return o; // end of statements
        } else {
            add_error(state, SYNTAXERROR, o->line, "failed to parse statement");
            ind += 1;
            body->subtokens = realloc_token(body->subtokens, (ind+1));
            body->subtoken_count = ind;
        }
    }
}

 // funcdef = type, identifier, "(", [type, identifier], {"," type,  identifier}, ")", "{"
Lextoken* parse_funcdef(Lextoken* p, Token* def){
    def->type = T_FUNCDEF;
    def->subtokens = init_token(p->line);
    def->subtokens = realloc_token(def->subtokens, 2);
    def->subtoken_count = 1;
    Lextoken* l = parse_type_void(p, def->subtokens);
    if(l == NULL){
        free(def->subtokens);
        def->subtoken_count = 0;
        return NULL;
    }
    if(!match(l, IDENTIFIER)){
        return NULL;
    }
    def->str = malloc(strlen(l->str)+1);
    memcpy(def->str, l->str, strlen(l->str)+1);
    l = next(l);
    if((!match(l, LEFT_PAREN)) || (l == NULL)){
        return NULL;
    }
    l = next(l);
    int j = 1;
    def->subtokens[j].type = T_VARPARAM;
    def->subtokens[j].subtokens = malloc(sizeof(struct Token));
    def->subtokens[j].subtoken_count = 1;
    def->subtoken_count = 2;
    Lextoken* ty1 = parse_type(l, (def->subtokens[j].subtokens));
    Lextoken* ident1 = match(ty1, IDENTIFIER) ? next(ty1) : NULL;
    
    if(ident1 != NULL){
        
        // identifier is ty1 
        def->subtokens[j].str = malloc(strlen(ty1->str)+1);
        memcpy(def->subtokens[j].str, ty1->str, strlen(ty1->str)+1);
        
        l = ident1;
        while(1){
            if(!match(l, COMMA)){
                break;
            }
            j += 1;
            def->subtokens = realloc(def->subtokens, (j+1)*sizeof(struct Token));
            def->subtoken_count = j+1;
            def->subtokens[j].type = T_VARPARAM;
            def->subtokens[j].subtokens = malloc(sizeof(struct Token));
            def->subtokens[j].subtoken_count = 1;
            Lextoken* ty = parse_type(next(l), def->subtokens[j].subtokens);
            Lextoken* ident = match(ty, IDENTIFIER) ? next(ty) : NULL;
            if(ident == NULL || ty == NULL){
                return NULL;
            }    
            def->subtokens[j].str = malloc(strlen(ty->str)+1);
            memcpy(def->subtokens[j].str, ty->str, strlen(ty->str)+1);
            l = ident;
        }
    } else {
        def->subtokens[j].subtoken_count = 0;
    }
    int jo = match(l, RIGHT_PAREN);
    jo = jo && match(next(l), LEFT_CRPAREN);
    if(jo){
        return next(next(l));
    }
    return NULL;
}

// function = funcdef, body, "}"
Lextoken* parse_function(Lextoken* p, Token* func, State* state){
    func->subtokens = init_token(p->line);
    func->subtokens = realloc_token(func->subtokens, 2);
    func->subtoken_count = 2;
    Lextoken* l = parse_funcdef(p, &func->subtokens[0]);
    if(l == NULL){
        destroy_token(func->subtokens);
        func->subtokens = NULL;
        return NULL;
    }
    l = parse_body(l, &func->subtokens[1], state);
    if(l != NULL && match(l, RIGHT_CRPAREN)){
        func->type = T_FUNCTION;
        return next(l);
    }
    destroy_token(func->subtokens);
    func->subtokens = NULL;
    return NULL;
}

// program = {[function | type] term }
Token* parse_program(Lextoken* p, State* state){
    Token* prog = init_token(p->line);
    prog->type = T_PROGRAM;
    prog->subtokens = init_token(p->line);
    int i = 0;
    while(1){
        if(match(p, TERM)){
            p = next(p);
            continue;
        }
        if(match(p, LEOF)){
            break;
        }
        Lextoken* l = parse_function(p, &prog->subtokens[i], state);
        if(l == NULL){
            l = parse_typedef(p, &prog->subtokens[i], state);
            if(l == NULL){
                add_error(state, SYNTAXERROR, p->line, "failed to parse function definition/body or type");
                return NULL;
            }
        }
        p = l;
        i += 1;
        prog->subtoken_count = i;
        prog->subtokens = realloc_token(prog->subtokens, (i+1));
    }
    if(state->verbose){
        print_tree(prog, 0);
    }
    return prog;
}


/**
 *
 *  misc funcs section
 * 
 * 
 */


