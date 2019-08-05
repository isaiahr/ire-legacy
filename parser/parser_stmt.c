#include<string.h>
#include<stdlib.h>
#include"parser.h"
#include"parseutils.h"
#include"parser_stmt.h"
#include"parser_expr.h"


// note: not a stmt on its own, but used in other stmts.
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

// varinit = type, identifier
Lextoken* parse_varinit(Lextoken* p, Token* e){
    e->subtokens = init_token(p->line);
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


// assignment = identifier, "=", expression
Lextoken* parse_assignment(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    i = i && match(next(p), EQUALS);
    e->subtokens = init_token(p->line);
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

// arrset = arrind, "=", expression
Lextoken* parse_arrset(Lextoken* p, Token* e){
    e->subtokens = init_token(p->line);
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
    e->subtokens = init_token(p->line);
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
    e->subtokens = init_token(p->line);
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


// setmember = accessor = expr
Lextoken* parse_setmember(Lextoken* p, Token* e){
    e->subtokens = init_token(p->line);
    p = parse_accessor(p, e->subtokens);
    if(p == NULL || (!match(p, EQUALS))){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->subtokens = realloc_token(e->subtokens, 2);
    p = parse_expression(next(p), &e->subtokens[1]);
    if(p == NULL){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->subtoken_count = 3;
    e->subtokens = realloc_token(e->subtokens, 3);
    
    // weird hack to split apart the accessor, to make it easy for semantic to proccess.
    int ind = 0;
    int lastdot = -1;
    while(e->subtokens[0].str[ind] != 0){
        if(e->subtokens[0].str[ind] == '.')
            lastdot = ind;
        ind += 1;
    }
    if(lastdot == -1){
        // corner case: one level of nesting. ("a.b")
        // "lift" the accessor subtoken onto setmember subtoken, to remove accessor.
        e->subtokens[2].type = T_MEMBERIDENT;
        e->subtokens[2].str = e->subtokens[0].str;
        e->type = T_SETMEMBER;
        memcpy(e->subtokens, e->subtokens[0].subtokens, sizeof(struct Token));
        return p;
    }
    e->subtokens[0].str[lastdot] = 0;
    e->subtokens[2].type = T_MEMBERIDENT;
    e->subtokens[2].str = &e->subtokens[0].str[lastdot+1];
    e->type = T_SETMEMBER;
    return p;
}

// if = if, expr, "{", body, "}"
Lextoken* parse_if(Lextoken* p, Token* e, State* state){
    if(!match(p, IF)){
        return NULL;
    }
    e->subtokens = init_token(p->line);
    Lextoken* a = parse_expression(next(p), &e->subtokens[0]);
    if(a == NULL || !match(a, LEFT_CRPAREN)){
        destroy_token(e->subtokens);
        return NULL;
    }
    e->subtokens = realloc_token(e->subtokens, 2);
    Lextoken* b = parse_body(next(a), &e->subtokens[1], state);
    if(!match(b, RIGHT_CRPAREN)){
        destroy_token(e->subtokens);
        return NULL;
    }
    e->type = T_IF;
    e->subtoken_count = 2;
    return next(b);
}

// statement = varinit | expression | assignment | return | arrset | setmember
Lextoken* parse_statement(Lextoken* p, Token* t, State* state){
    Lextoken* l = parse_varinit(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_assignment(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_return(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_addeq(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_arrset(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_setmember(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_if(p, t, state);
    if(l != NULL){
        return l;
    }
    return parse_expression(p, t);
}
