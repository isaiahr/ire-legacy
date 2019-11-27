#include<string.h>
#include<stdlib.h>
#include"parser.h"
#include"parseutils.h"
#include"parser_expr.h"
#include"parser_arith.h"
#include"parser_stmt.h"


// constant = (["-"], int) | char | ("true" | "false")
Lextoken* parse_constant(Lextoken* p, Token* e){
    if(match(p, TRUE)){
        e->type = T_BOOLEAN;
        e->lnt = 1;
        return next(p);
    }
    if(match(p, FALSE)){
        e->type = T_BOOLEAN;
        e->lnt = 0;
        return next(p);
    }
    int i = match(p, SUBTRACT);
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
    e->type = T_VARIABLE;
    e->str = malloc(strlen(p->str)+1);
    memcpy(e->str, p->str, strlen(p->str)+1);
    return next(p);
}

// expression = (constant | string | variable | funccall | arrind)
Lextoken* parse_expression(Lextoken* p, Token* e){
    return parse_expression_flags(p, e, 0);
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
    Token* child = allocate_child_token(e, p->line);
    Lextoken* l = parse_expression(next(next(p)), child);
    if(l == NULL){
        i = i && match(next(next(p)), RIGHT_PAREN);
        if(i){
            destroy_children(e);
            return next(next(next(p)));
        }
        destroy_children(e);
        free(e->str);
        return NULL;
    }
    
    i = 1;
    while(l != NULL){
        if(match(l, COMMA)){
            Token* chil = allocate_child_token(e, l->line);
            l = parse_expression(next(l), chil);
            if(l == NULL){
                free(e->str);
                destroy_children(e);
                return NULL;
            }
        }
        else {
            if(match(l, RIGHT_PAREN)){
                return next(l);
            }
            destroy_children(e);
            free(e->str);
            return NULL;
        }
        i += 1;
    }
    destroy_children(e);
    free(e->str);
    return NULL; // not possible (?)
}


Lextoken* parse_arrind(Lextoken* p, Token* e){
    return parse_arrind_flags(p, e, 0);
}

// arrind = expression "[", expression, "]"
// to avoid infinite left recursion, parse as
// arrind = exprnoarrind "[", expression, "]", {"[", expression, "]" }
// this _should_ be equivalent. 
Lextoken* parse_arrind_flags(Lextoken* p, Token* e, int FLAGS){
    Token* left = init_token(p->line);
    Token* right = init_token(p->line);
    Lextoken* expr = parse_expression_flags(p, left, FLAG_ARRIND | FLAGS);
    if(expr == NULL){
        destroy_token(left);
        destroy_token(right);
        return NULL;
    }
    // a = "has at least one match"
    int a = 0;
    while(1){
        int j = match(expr, LEFT_SQPAREN);
        if((!a) && ((!j) || (expr == NULL))){
            destroy_children(e);
            return NULL;
        }
        Lextoken* l = j ? parse_expression(next(expr), right) : NULL;
        if(j && (l != NULL) && match(l, RIGHT_SQPAREN)){
            a = 1;
            Token* newl = init_token(p->line);
            adopt_child_token(newl, left);
            adopt_child_token(newl, right);
            newl->type = T_INDGET;
            left = newl;
            expr = next(l);
        }
        else{
            if (a) {
                // good.
                e->type = T_INDGET;
                e->subtokens = left->subtokens;
                e->line = left->line;
                return expr;
            }
            destroy_children(e);
            return NULL;
        }
    }
}

// card = "||",  expression,  "||"
Lextoken* parse_card(Lextoken* p, Token* e){
    if(!match(p, DOUBLEPIPE)){
        return NULL;
    }
    Token* child = allocate_child_token(e, p->line);
    e->type = T_CARDINALITY;
    Lextoken* a = parse_expression(next(p), child);
    if(a == NULL || !match(a, DOUBLEPIPE)){
        destroy_children(e);
        return NULL;
    }
    return next(a);
}

/// newarr = new, type, "[", expr, "]"
Lextoken* parse_newarr(Lextoken* p, Token* e){
    if(!match(p, NEW)){
        return NULL;
    }
    Token* c1 = allocate_child_token(e, p->line);
    Token* c2 = allocate_child_token(e, p->line);
    Lextoken* l = parse_type(next(p), c1);
    if(l == NULL || (!match(l, LEFT_SQPAREN))){
        destroy_children(e);
        return NULL;
    }
    Lextoken* l2 = parse_expression(next(l), c2);
    if(l2 == NULL || (!match(l2, RIGHT_SQPAREN))){
        destroy_children(e);
        return NULL;
    }
    // good
    e->type = T_NEWARR;
    return next(l2);
}

// inv = !expr
Lextoken* parse_inv(Lextoken* p, Token* e){
    if(!match(p, EXCLAMATION)){
        return NULL;
    }
    Token* child = allocate_child_token(e, p->line);
    Lextoken* l = parse_expression(next(p), child);
    if(l == NULL){
        destroy_children(e);
        return NULL;
    }
    e->type = T_INVERT;
    return l;
}

// brexpr = "(", expression, ")"
Lextoken* parse_brexpr(Lextoken* p, Token* e){
    if(!match(p, LEFT_PAREN)){
        return NULL;
    }
    Lextoken* o = parse_expression(next(p), e);
    if(o == NULL || !match(o, RIGHT_PAREN)){
        return NULL;
    }
    return next(o);
}

// accessor = expr "." identifier { ".", identifier }
Lextoken* parse_accessor(Lextoken* p, Token* e){
    return parse_accessor_flags(p, e, 0);
}

Lextoken* parse_accessor_flags(Lextoken* p, Token* e, int FLAGS){
    Token* child = allocate_child_token(e, p->line);
    p = parse_expression_flags(p, child, FLAGS | FLAG_ACCESSOR);
    if(p == NULL){
        destroy_children(e);
        return NULL;
    }
    if(!(match(p, DOT) && match(next(p), IDENTIFIER))){
        destroy_children(e);
        return NULL;
    }
    e->str = malloc(strlen(next(p)->str)+1);
    memcpy(e->str, next(p)->str, strlen(next(p)->str)+1);
    p = next(next(p));
    while(match(p, DOT)){
        if(!match(next(p), IDENTIFIER)){
            // bad
            destroy_children(e);
            return NULL;
        }
        char* old = e->str;
        e->str = malloc(strlen(next(p)->str)+strlen(e->str)+2);
        memcpy(e->str, old, strlen(old));
        memcpy(&e->str[strlen(old)], ".", 1);
        memcpy(&e->str[strlen(old)+1], next(p)->str, strlen(next(p)->str)+1);
        p = next(next(p));
    }
    e->type = T_ACCESSOR;
    return p;
}

// gettag = expression, ":", identifier
Lextoken* parse_gettag(Lextoken* p, Token* e){
    return parse_gettag_flags(p, e, 0);
}

// note: expression ":" identifier will be boolean, so
// expression ":" expression ":" identifier is valid semantically.
// it could be valid syntatically, but since it isnt ever valid semantically
// we dont parse for it. (you can achieve the same thing by using brackets)
Lextoken* parse_gettag_flags(Lextoken* p, Token* e, int FLAGS){
    Token* child = allocate_child_token(e, p->line);
    p = parse_expression_flags(p, child, FLAGS | FLAG_GETTAG);
    if(p == NULL || !match(p, COLON) || !match(next(p), IDENTIFIER)){
        destroy_children(e);
        return NULL;
    }
    // ok.
    e->str = malloc(strlen(next(p)->str) + 1);
    memcpy(e->str, next(p)->str, strlen(next(p)->str));
    e->str[strlen(next(p)->str)] = 0;
    e->type = T_GETTAG;
    return next(next(p));
}


// constructor = new, type, segconstruct, {&, segconstruct}
Lextoken* parse_constructor(Lextoken* p, Token* e){
    if(!match(p, NEW)){
        return NULL;
    }
    Token* child = allocate_child_token(e, p->line);
    p = parse_type(next(p), child);
    if(p == NULL){
        destroy_children(e);
        return NULL;
    }
    Token* ch2 = allocate_child_token(e, p->line);
    p = parse_segconstruct(p, ch2);
    if(p == NULL){
        destroy_children(e);
        return NULL;
    }
    int ind = 2;
    while(1){
        if(!match(p, AMPERSAND)){
            // ok.
            e->type = T_CONSTRUCTOR;
            return p;
        }
        Token* y = allocate_child_token(e, p->line);
        p = parse_segconstruct(next(p), y);
        if(p == NULL){
            // "&", then not segconstruct. fail.
            destroy_children(e);
            return NULL;
        }
        ind += 1;
    }
}

// segconstruct = [identifier, { "." , identifier }] "(", {identifier, "=", expression ","} ")"
Lextoken* parse_segconstruct(Lextoken* p, Token* e){
    // count str to get
    int sz = 0;
    Lextoken* o = p;
    if(match(p, IDENTIFIER)){
        int num = 1;
        sz += strlen(p->str);
        p = next(p);
        while(match(p, DOT)){
            if(!match(next(p), IDENTIFIER)){
                // bad.
                return NULL; 
            }
            num += 1;
            sz += strlen(next(p)->str)+1; // (for dot)
            p = next(next(p));
        }
        e->str = malloc(sz+1);
        e->str[0] = 0;
        for(int i = 0; i < num; i++){
            strcat(e->str, o->str);
            if(i+1 < num){
                strcat(e->str, ".");
                o = next(next(o));
            }
        }
        o = next(o);
    }
    // op = "original p" (possibly. its next token.)
    if(!match(o, LEFT_PAREN)){
        return NULL; // free str possibly? (no)
    }
    if(match(next(o), RIGHT_PAREN)){
        // empty. ok.
        return next(next(o));
    }
    o = next(o);
    Token* child = allocate_child_token(e, o->line);
    while(1){
        if(!(match(o, IDENTIFIER) && match(next(o), EQUALS))){
            destroy_children(e);
            return NULL;
        }
        child->str = malloc(strlen(o->str)+1);
        child->type = T_CONSTRUCTASSIGN;
        memcpy(child->str, o->str, strlen(o->str)+1);
        Token* grandchild = allocate_child_token(child, o->line);
        o = parse_expression(next(next(o)), grandchild);
        if(o == NULL){
            // invalid.
            // NOTE: previous note is wrong probably
            destroy_children(e);
            return NULL;
        }
        if(!match(o, COMMA)){
            break;
        }
        child = allocate_child_token(e, o->line);
        o = next(o);
    }
    if(!match(o, RIGHT_PAREN)){
        destroy_children(e);
        return NULL;
    }
    e->type = T_SEGCONSTRUCT;
    return next(o);
}

// expr with flags telling it what not to parse.
// this is to avoid infinite left recursion.
Lextoken* parse_expression_flags(Lextoken* p, Token* e, int FLAGS){
    Lextoken* l = NULL;
    if((!(FLAGS & FLAG_ARITH)) && (l = parse_arith_flags(p, e, FLAGS))){
        return l;
    }
    if((!(FLAGS & FLAG_GETTAG)) && (l = parse_gettag_flags(p, e, FLAGS))){
        return l;
    }
    if((l = parse_funcall(p, e))){
        return l;
    }
    if((l = parse_constant(p, e))){
        return l;
    }
    if((l = parse_card(p, e))){
        return l;
    }    
    if((l = parse_newarr(p, e))){
        return l;
    }
    if((l = parse_inv(p, e))){
        return l;
    }
    if((!(FLAGS & FLAG_ARRIND)) && (l = parse_arrind_flags(p, e, FLAGS))){
        return l;
    }
    if((l = parse_constructor(p, e))){
        return l;
    }
    if((!(FLAGS & FLAG_ACCESSOR)) && (l = parse_accessor_flags(p, e, FLAGS))){
        return l;
    }
    if(match(p, LSTRING)){
        e->type = T_STRING;
        e->str = malloc(strlen(p->str)+1);
        memcpy(e->str, p->str, strlen(p->str)+1);
        return next(p);
    }
    
    // ordering important here. both brexpr and arith can pass on (a+b)*c.
    // however (a+b) will fail later.
    if((l = parse_brexpr(p, e))){
        return l;
    }
    return parse_variable(p, e);
}
