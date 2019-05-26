#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"common.h"
#include"error.h"
#include"lexer.h"
#include"parser.h"


// THIS IS THE FIFTH TIME THE PARSER HAS BEEN REWRITTEN. and hopefully the last.

/**
 * EBNF OF SYNTAX
 * 
 *  constant = (["-"], int) | char
 * variable = identifier
 * type = indentifier, [ "[", "]"]
 * expression = (constant | string | variable | funccall)
 * varinit = type, identifier
 * funccall = identifier, "(", [expression {",", expression } ] ")"
 * assignment = identifier, "=", expression
 * arrind = expression [expression]
 * arrset = expression [expression], "=", expression
 * addeq = expression, "+=", expression
 * return = return, expression
 * statement = varinit | expression | assignment | return
 * body = {[statement], term}
 * funcdef = type, identifier, "(", [type, identifier], {"," type,  identifier}, ")", "{"
 * function = funcdef, body, "}"
 * program = {[function] term }

 *
 * 
 */

/***
 * 
 * Parsing is done by recursive descent using the syntax tree shown above.
 * 
 * 
 */
int match(Lextoken* p, int m){
    if(p == NULL){
        return 0;
    }
    int i = p->type == m;
    p = p->next;
    return i;
}

Lextoken* next(Lextoken* p){
    if(p == NULL){
        return NULL;
    }
    return p->next;
}

// constant = (["-"], int) | char
Lextoken* parse_constant(Lextoken* p, Token* e){
    int i = match(p, MINUS_SYM);
    i = i && match(next(p), INTEGER);
    if(!i){
        int i = match(p, LCHAR);
        if(i){
            e->type = T_CHAR;
            e->chr = p->chr;
            return next(p);
        }
        else{
            if (match(p, INTEGER)){
                e->type = T_INT;
                e->lnt = p->lnt;
                return next(p);
            }
        }
    }
    else{
        e->type = T_INT;
        e->lnt = p->lnt;
        return next(next(p));
    }
    return NULL;
}

// variable = identifier, ["[",expression,"]" ]
Lextoken* parse_variable(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    if(!i){
        return NULL;
    }
    e->subtoken_count = 0;
    e->type = T_VARIABLE;
    e->str = malloc(strlen(p->str)+1);
    memcpy(e->str, p->str, strlen(p->str)+1);
    return next(p);
}

// type = indentifier, [ "[", "]"]
Lextoken* parse_type(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    if(!i){
        return NULL;
    }
    e->type = T_TYPE;
    e->str = malloc(strlen(p->str)+3);
    memcpy(e->str, p->str, strlen(p->str)+1);
    int j = match(next(p), LEFT_SQPAREN);
    j = j && match(next(next(p)), RIGHT_SQPAREN);
    if(j){
        e->str[strlen(p->str)] = '[';
        e->str[strlen(p->str)+1] = ']';
        e->str[strlen(p->str)+2] = 0;
        return next(next(next(p)));
    }
    e->lnt = 0;
    return next(p);
}

// expression = (constant | string | variable | funccall | arrind)
Lextoken* parse_expression(Lextoken* p, Token* e){
    Lextoken* l = NULL;
    if((l = parse_funcall(p, e))){
        return l;
    }
    if((l = parse_constant(p, e))){
        return l;
    }
    if((l = parse_arrind(p, e))){
        return l;
    }
    if(match(p, LSTRING)){
        e->type = T_STRING;
        e->str = malloc(strlen(p->str)+1);
        memcpy(e->str, p->str, strlen(p->str)+1);
        return next(p);
    }
    return parse_variable(p, e);
}

// varinit = type, identifier
Lextoken* parse_varinit(Lextoken* p, Token* e){
    e->subtokens = init_token();
    e->subtoken_count = 1;
    Lextoken* l = parse_type(p, e->subtokens);
    if(match(l, IDENTIFIER)){
        e->str = malloc(strlen(l->str)+1);
        memcpy(e->str, l->str, strlen(l->str)+1);
        e->type = T_VARINIT;
        return next(l);
    }
    e->subtoken_count = 0;
    destroy_token(e->subtokens);
    e->subtokens = NULL;
    return NULL;
}

// funccall = identifier, "(", [expression {",", expression } ] ")"
Lextoken* parse_funcall(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    i = i && match(next(p), LEFT_PAREN);
    if(!i){
        return NULL;
    }
    e->type = T_FUNCALL;
    e->str = malloc(strlen(p->str) + 1);
    memcpy(e->str, p->str, strlen(p->str)+1);
    e->subtokens = init_token();
    Lextoken* l = parse_expression(next(next(p)), e->subtokens);
    if(l == NULL){
        i = i && match(next(next(p)), RIGHT_PAREN);
        if(i){
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            return next(next(next(p)));
        }
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        free(e->str);
        return NULL;
    }
    
    e->subtoken_count = 1;
    i = 1;
    while(l != NULL){
        if(match(l, COMMA)){
            e->subtokens = realloc_token(e->subtokens, (i+1));
            e->subtoken_count = i+1;
            l = parse_expression(next(l), &e->subtokens[i]);
            if(l == NULL){
                free(e->str);
                destroy_token(e->subtokens);
                e->subtokens = NULL;
                return NULL;
            }
        }
        else {
            if(match(l, RIGHT_PAREN)){
                return next(l);
            }
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            free(e->str);
            return NULL;
        }
    }
    destroy_token(e->subtokens);
    e->subtokens = NULL;
    free(e->str);
    return NULL; // not possible (?)
}


// assignment = identifier, "=", expression
Lextoken* parse_assignment(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    i = i && match(next(p), EQUALS);
    e->subtokens = init_token();
    e->subtoken_count = 1;
    Lextoken* l = parse_expression(next(next(p)), e->subtokens);
    if(i && l != NULL){
        e->type = T_ASSIGNMENT;
        e->str = malloc(strlen(p->str)+1);
        memcpy(e->str, p->str, strlen(p->str)+1);
        return l;
    }
    destroy_token(e->subtokens);
    e->subtokens = NULL;
    return NULL;
}

// helper, see below
Lextoken* parse_expression_noarr(Lextoken* p, Token* e){
    Lextoken* l = NULL;
    if((l = parse_funcall(p, e))){
        return l;
    }
    if((l = parse_constant(p, e))){
        return l;
    }
    if(match(p, LSTRING)){
        e->type = T_STRING;
        e->str = malloc(strlen(p->str)+1);
        memcpy(e->str, p->str, strlen(p->str)+1);
        return next(p);
    }
    return parse_variable(p, e);
}

// arrind = expression "[", expression, "]"
// to avoid infinite left recursion, parse as
// arrind = exprnoarrind "[", expression, "]", {"[", expression, "]" }
// this _should_ be equivalent. 
Lextoken* parse_arrind(Lextoken* p, Token* e){
    // despite the name its left and right.
    Token* left = init_token();
    left = realloc_token(left, 2);
    Lextoken* expr = parse_expression_noarr(p, left);
    int a = 0;
    while(1){
        int j = match(expr, LEFT_SQPAREN);
        if((!a) && ((!j) || (expr == NULL))){
            destroy_token(left);
            return NULL;
        }
        Lextoken* l = parse_expression(next(expr), &left[1]);
        if(j && (l != NULL) && match(l, RIGHT_SQPAREN)){
            a = 1;
            e->subtoken_count = 2;
            e->subtokens = left;
            e->type = T_INDGET;
            left = init_token();
            left = realloc_token(left, 2);
            left->subtokens = e->subtokens;
            left->subtoken_count = 2;
            left->type = T_INDGET;
            expr = next(l);
        }
        else{
            if (a) {
                // good.
                return expr;
            }
            destroy_token(left);
            e->subtoken_count = 0;
            e->subtokens = NULL;
            return NULL;
        }
    }
}

// arrset = arrind, "=", expression
Lextoken* parse_arrset(Lextoken* p, Token* e){
    e->subtokens = init_token();
    e->subtokens = realloc_token(e->subtokens, 2);
    e->subtoken_count = 2;
    Lextoken* a = parse_arrind(p, e->subtokens);
    if(match(a, EQUALS)){
        Lextoken* p = parse_expression(next(a), &e->subtokens[1]);
        if(p != NULL){
            e->type = T_INDSET;
            return p;
        }
    }
    destroy_token(e->subtokens);
    e->subtoken_count = 0;
    return NULL;
}

// addeq = expression, "+=", expression
Lextoken* parse_addeq(Lextoken* p, Token* e){
    e->subtokens = init_token();
    e->subtokens = realloc_token(e->subtokens, 2);
    e->subtoken_count = 2;
    Lextoken* a = parse_expression(p, e->subtokens);
    if(match(a, ADDEQ)){
        Lextoken* p = parse_expression(next(a), &e->subtokens[1]);
        if(p != NULL){
            e->type = T_ADDEQ;
            return p;
        }
    }
    destroy_token(e->subtokens);
    e->subtoken_count = 0;
    return NULL;
}

// return = return, expression
Lextoken* parse_return(Lextoken* p, Token* e){
    if(!match(p, RETURN)){
        return NULL;
    }
    e->subtokens = init_token();
    e->subtoken_count = 1;
    Lextoken* k = next(p);
    Lextoken* a = parse_expression(k, e->subtokens);
    if(a == NULL){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->type = T_RETURN;
    return a;
}

// statement = varinit | expression | assignment | return | arrset
Lextoken* parse_statement(Lextoken* p, Token* e){
    Lextoken* l = parse_varinit(p, e);
    if(l != NULL){
        return l;
    }
    l = parse_assignment(p, e);
    if(l != NULL){
        return l;
    }
    l = parse_return(p, e);
    if(l != NULL){
        return l;
    }
    l = parse_addeq(p, e);
    if(l != NULL){
        return l;
    }
    l = parse_arrset(p, e);
    if(l != NULL){
        return l;
    }
    return parse_expression(p, e);
}

// body = {[statement], term}
Lextoken* parse_body(Lextoken* p, Token* body){
    int ind = 0;
    body->subtokens = init_token();
    body->type = T_BODY;
    while(1){
        Lextoken* o = p;
        p = parse_statement(p, &body->subtokens[ind]);
        if(p != NULL){
            if(match(p, TERM)){
                // good
                ind += 1;
                body->subtokens = realloc_token(body->subtokens, (ind+1));
                body->subtoken_count = ind;
                p = next(p);
            }
            else{
                return NULL; // statement with no term
            }
            
        }
        else if(match(o, TERM)){
            p = next(o); // empty stmt
        }
        else{
            return o; // end of statements
        }
    }
}

 // funcdef = type, identifier, "(", [type, identifier], {"," type,  identifier}, ")", "{"
Lextoken* parse_funcdef(Lextoken* p, Token* def){
    def->type = T_FUNCDEF;
    def->subtokens = init_token();
    def->subtokens = realloc_token(def->subtokens, 2);
    def->subtoken_count = 1;
    Lextoken* l = parse_type(p, def->subtokens);
    if(l == NULL){
        free(def->subtokens);
        def->subtoken_count = 0;
        return NULL;
    }
    def->str = malloc(strlen(l->str)+1);
    memcpy(def->str, l->str, strlen(l->str)+1);
    l = match(l, IDENTIFIER) ? next(l) : NULL;
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
Lextoken* parse_function(Lextoken* p, Token* func){
    func->subtokens = init_token();
    func->subtokens = realloc_token(func->subtokens, 2);
    func->subtoken_count = 2;
    Lextoken* l = parse_funcdef(p, &func->subtokens[0]);
    if(l == NULL){
        destroy_token(func->subtokens);
        func->subtokens = NULL;
        return NULL;
    }
    l = parse_body(l, &func->subtokens[1]);
    if(l != NULL && match(l, RIGHT_CRPAREN)){
        func->type = T_FUNCTION;
        return next(l);
    }
    destroy_token(func->subtokens);
    func->subtokens = NULL;
    return NULL;
}

// program = {[function] term }
Token* parse_program(Lextoken* p){
    Token* prog = init_token();
    prog->type = T_PROGRAM;
    prog->subtokens = init_token();
    int i = 0;
    while(1){
        Lextoken* l = parse_function(p, &prog->subtokens[i]);
        if(l != NULL){
            p = l;
        }
        if(!match(p, TERM)){
            break;
        }
        else if (l == NULL){
            p = next(p);
            continue;
        }
        i += 1;
        prog->subtoken_count = i;
        prog->subtokens = realloc_token(prog->subtokens, (i+1));
        p = next(p);
    }
    print_tree(prog, 0);
    return prog;
}

Token* init_token(){
    Token* t = malloc(sizeof(struct Token));
    t->str = NULL;
    t->subtoken_count = 0;
    t->lnt = 0;
    t->subtokens = NULL;
    t->chr = 0;
    return t;
}

Token* realloc_token(Token* ptr, int len){
    Token* pt = realloc(ptr, len*sizeof(struct Token));
    pt[len-1].str = NULL;
    pt[len-1].subtoken_count = 0;
    pt[len-1].lnt = 0;
    pt[len-1].subtokens = NULL;
    pt[len-1].chr = 0;
    return pt;
}

void destroy_token(Token* ptr){
    return; // TODO make this work in the future.
    if(ptr == NULL){
        return;
    }
    for(int i = 0; (i*sizeof(struct Token)) < sizeof(ptr); i++){
        if(ptr[i].str != NULL){
            free(ptr[i].str);
            ptr[i].str = NULL;
        }
        if(ptr[i].subtokens != NULL){
            destroy_token(ptr[i].subtokens);
            ptr[i].subtokens = NULL;
        }
    }
    free(ptr);
}

void print_tree(Token* p, int lvl){
    char* lvlstr = malloc(lvl+1);
    for(int i = 0; i < lvl; i++){
        lvlstr[i] = ' ';
    }
    lvlstr[lvl] = 0;
    if(p->str == NULL){
        printf("%s%s\n", lvlstr, type(p));
    } else {
        printf("%s%s [%s]\n", lvlstr, type(p), p->str);
    }
    free(lvlstr);
    if(p->subtokens != NULL){
        for(int i = 0; i < p->subtoken_count; i++){
            print_tree(&p->subtokens[i], lvl+4);
        }
    }
}


char* type(Token* p){
    switch(p->type){
        case T_PROGRAM: return "PROGRAM";
        case T_FUNCTION: return "FUNCTION";
        case T_ASSIGNMENT: return "ASSIGNMENT";
        case T_TYPE: return "TYPE";
        case T_VARIABLE: return "VARIABLE";
        case T_CHAR: return "CHAR";
        case T_INT: return "INT";
        case T_STRING: return "STRING";
        case T_VARINIT: return "VARINIT";
        case T_FUNCDEF: return "FUNCDEF";
        case T_VARPARAM: return "VARPARAM";
        case T_BODY: return "BODY";
        case T_RETURN: return "RETURN";
        case T_FUNCALL: return "FUNCALL";
        case T_INDGET: return "INDGET";
        case T_INDSET: return "INDSET";
        case T_ADDEQ: return "ADDEQ";
        default: return "UNKNOWN";
    }
    
}
