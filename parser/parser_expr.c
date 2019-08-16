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
    e->subtoken_count = 0;
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
    e->subtokens = init_token(p->line);
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
        i += 1;
    }
    destroy_token(e->subtokens);
    e->subtokens = NULL;
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
    // despite the name its left and right.
    Token* left = init_token(p->line);
    left = realloc_token(left, 2);
    Lextoken* expr = parse_expression_flags(p, left, FLAG_ARRIND | FLAGS);
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
            left = init_token(p->line);
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

// card = "||",  expression,  "||"
Lextoken* parse_card(Lextoken* p, Token* e){
    if(!match(p, DOUBLEPIPE)){
        return NULL;
    }
    e->subtokens = init_token(p->line);
    e->type = T_CARDINALITY;
    e->subtoken_count = 1;
    Lextoken* a = parse_expression(next(p), e->subtokens);
    if(a == NULL || !match(a, DOUBLEPIPE)){
        destroy_token(e);
        return NULL;
    }
    return next(a);
}

/// newarr = new, type, "[", expr, "]"
Lextoken* parse_newarr(Lextoken* p, Token* e){
    if(!match(p, NEW)){
        return NULL;
    }
    e->subtokens = init_token(p->line);
    e->subtokens = realloc_token(e->subtokens, 2);
    e->subtoken_count = 2;
    Lextoken* l = parse_type(next(p), e->subtokens);
    if(l == NULL || (!match(l, LEFT_SQPAREN))){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        e->subtoken_count = 0;
        return NULL;
    }
    Lextoken* l2 = parse_expression(next(l), &e->subtokens[1]);
    if(l2 == NULL || (!match(l2, RIGHT_SQPAREN))){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        e->subtoken_count = 0;
        return NULL;
    }
    // good
    e->type = T_NEWARR;
    return next(l2);
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
    e->subtokens = init_token(p->line);
    p = parse_expression_flags(p, e->subtokens, FLAGS | FLAG_ACCESSOR);
    if(p == NULL){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    if(!(match(p, DOT) && match(next(p), IDENTIFIER))){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->str = malloc(strlen(next(p)->str)+1);
    memcpy(e->str, next(p)->str, strlen(next(p)->str)+1);
    p = next(next(p));
    while(match(p, DOT)){
        if(!match(next(p), IDENTIFIER)){
            // bad
            destroy_token(e->subtokens);
            e->subtokens = NULL;
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
    e->subtoken_count = 1;
    return p;
}


// constructor = new, type, segconstruct, {&, segconstruct}
Lextoken* parse_constructor(Lextoken* p, Token* e){
    if(!match(p, NEW)){
        return NULL;
    }
    e->subtokens = init_token(p->line);
    p = parse_type(next(p), e->subtokens);
    if(p == NULL){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->subtokens = realloc_token(e->subtokens, 2);
    p = parse_segconstruct(p, &e->subtokens[1]);
    if(p == NULL){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    int ind = 2;
    while(1){
        if(!match(p, AMPERSAND)){
            // ok.
            e->subtoken_count = ind;
            e->type = T_CONSTRUCTOR;
            return p;
        }
        e->subtokens = realloc_token(e->subtokens, ind+1);
        p = parse_segconstruct(next(p), &e->subtokens[ind]);
        if(p == NULL){
            // "&", then not segconstruct. fail.
            destroy_token(e->subtokens);
            e->subtokens = NULL;
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
    e->subtokens = init_token(o->line);
    int ind = 0;
    while(1){
        if(!(match(o, IDENTIFIER) && match(next(o), EQUALS))){
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            return NULL;
        }
        e->subtokens[ind].str = malloc(strlen(o->str)+1);
        e->subtokens[ind].type = T_CONSTRUCTASSIGN;
        memcpy(e->subtokens[ind].str, o->str, strlen(o->str)+1);
        e->subtokens[ind].subtokens = init_token(o->line);
        o = parse_expression(next(next(o)), e->subtokens[ind].subtokens);
        if(o == NULL){
            // invalid.
            // NOTE. not destroying e->subtokens[x].subtokens.
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            return NULL;
        }
        e->subtokens[ind].subtoken_count = 1;
        if(!match(o, COMMA)){
            break;
        }
        ind += 1;
        e->subtokens = realloc_token(e->subtokens, ind+1);
        o = next(o);
    }
    if(!match(o, RIGHT_PAREN)){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->subtoken_count = ind+1;
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
