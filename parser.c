#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"common.h"
#include"error.h"
#include"lexer.h"
#include"parser.h"

#define FLAG_ARRIND 1
#define FLAG_ARITH 2


int match(Lextoken* p, int m);
Lextoken* next(Lextoken* p);
Lextoken* parse_constant(Lextoken* p, Token* t);
Lextoken* parse_variable(Lextoken* p, Token* t);
Lextoken* parse_type(Lextoken* p, Token* t);
Lextoken* parse_expression(Lextoken* p, Token* t);
Lextoken* parse_expression_flags(Lextoken* p, Token* t, int flags);
Lextoken* parse_varinit(Lextoken* p, Token* t);
Lextoken* parse_funcall(Lextoken* p, Token* t);
Lextoken* parse_arrind(Lextoken* p, Token* e);
Lextoken* parse_arrind_flags(Lextoken* p, Token* e, int flags);
Lextoken* parse_arrset(Lextoken* p, Token* e);
Lextoken* parse_addeq(Lextoken* p, Token* e);
Lextoken* parse_card(Lextoken* p, Token* e);
Lextoken* parse_newarr(Lextoken* p, Token* e);
Lextoken* parse_arith(Lextoken* p, Token* e);
Lextoken* parse_arith_flags(Lextoken* p, Token* e, int flags);
Lextoken* parse_brexpr(Lextoken* p, Token* e);
Lextoken* parse_assignment(Lextoken* p, Token* t);
Lextoken* parse_statement(Lextoken* p, Token* t);
Lextoken* parse_body(Lextoken* p, Token* t, State* state);
Lextoken* parse_funcdef(Lextoken* p, Token* t);
Lextoken* parse_function(Lextoken* p, Token* t, State* state);
void print_tree(Token* p, int lvl);
char* type(Token* t);
Token* init_token();
Token* realloc_token(Token* ptr, int len);
void destroy_token(Token* ptr);

// THIS IS THE FIFTH TIME THE PARSER HAS BEEN REWRITTEN. and hopefully the last.

/**
 * EBNF OF SYNTAX
 * 
 *  This is outdated. see comments before funcs for more up to date.
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
    return parse_expression_flags(p, e, 0);
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

// card = "|",  expression,  "|"
Lextoken* parse_card(Lextoken* p, Token* e){
    if(!match(p, PIPE)){
        return NULL;
    }
    e->subtokens = init_token(p->line);
    e->type = T_CARDINALITY;
    e->subtoken_count = 1;
    Lextoken* a = parse_expression(next(p), e->subtokens);
    if(a == NULL || !match(a, PIPE)){
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

// arith = exprnoarith ("<" | ">" | "=" | "+" | "-") exprnoarith
/**
* Algorithmn details: O(n) n = length of sentence (also scales with symbols)
*/

typedef struct ArithExpr{
    Token* p;
    struct OpExpr* next;
    struct OpExpr* prev;
} ArithExpr;

typedef struct OpExpr {
    int type;
    struct ArithExpr* next;
    struct ArithExpr* prev;
} OpExpr;

OpExpr* opexprhelper(Lextoken* p){
    switch(p->type){
        case PLUS:
        case DOUBLEEQUALS:
        case LESS:
        case GREATER:
        case SUBTRACT:
        case MULT:
            ; // nessecary
            OpExpr* o = malloc(sizeof(struct OpExpr));
            o->type = p->type;
            o->next = NULL;
            o->prev = NULL;
            return o;
        default: return NULL;
    }
}

#define LAST_ORDER 3

/**
 *
 * Note: lower order = strong associativity. (or higher order)
 * higher orders "wait" for lower orders to compute before computing.
 * a expr like a == b * c + d
 * will be (a == ((b*c)+d))
 * 
 */

int ordermatches (int order, int type){
    switch(type){
        case MULT:
            return order == 0;
        case PLUS:
        case SUBTRACT:
            return order == 1;
        case LESS:
        case GREATER:
            return order == 2;
        case DOUBLEEQUALS:
            return order == 3;
        default:
            return 0;
    }
    return 0; // shouldnt happen
}

Lextoken* parse_arith(Lextoken* p, Token* e){
    return parse_arith_flags(p, e, 0);
}

Lextoken* parse_arith_flags(Lextoken* p, Token* e, int flags){
    // 1. build the initial list.
    ArithExpr* head = malloc(sizeof(struct ArithExpr));
    Token* o = init_token(p->line);
    Lextoken* l = parse_expression_flags(p, o, FLAG_ARITH | flags);
    if(l == NULL){
        destroy_token(o);
        free(head);
        return NULL;
    }
    head->p = o;
    head->next = opexprhelper(l);
    head->prev = NULL;
    if(head->next == NULL){
        destroy_token(o);
        free(head);
        return NULL;
    }
    head->next->next = malloc(sizeof(struct ArithExpr));
    o = init_token(p->line);
    l = parse_expression_flags(next(l), o, FLAG_ARITH);
    if(l == NULL){
        destroy_token(head->p);
        destroy_token(o);
        free(head);
        free(head->next);
        free(head->next->next);
        return NULL;
    }
    head->next->next->p = o;
    head->next->next->next = NULL;
    head->next->next->prev = head->next;
    head->next->prev = head;
    // valid.
    ArithExpr* cur = head->next->next;
    while(1){
        OpExpr* op = opexprhelper(l);
        if(op == NULL){
            //ok. proceed.
            break;
        }
        op->next = NULL;
        cur->next = op;
        cur->next->prev = cur;
        Token* newt = init_token(next(l)->line);
        l = parse_expression_flags(next(l), newt, FLAG_ARITH);
        if(l == NULL){
            // failed, no expr on end (such as "3*2*")
            // gc all held items
            int y = 1;
            void* hed = (void*) head;
            while(head != NULL){
                if(y){
                    ArithExpr* ae = (ArithExpr*) hed;
                    destroy_token(ae->p);
                    free(ae);
                    hed = (void*) ae->next;
                }
                else{
                    OpExpr* oe = (OpExpr*) hed;
                    free(oe);
                    hed = (void*) oe->next;
                }
                y = !y;
            }
            return NULL;
        }
        cur->next->next = malloc(sizeof(struct ArithExpr*));
        cur->next->next->prev = cur->next;
        cur->next->next->next = NULL;
        cur->next->next->p = newt;
        cur = cur->next->next;
        // ok. proceed.
    }
    // 2. parse flat tree to account for order of operations.
    // lower order = first.
    for(int order = 0; order <= LAST_ORDER; order++){
        ArithExpr* cur = head;
        while(cur->next != NULL){
            // note cur->next nonull implies cur->next->next exists.
            if(ordermatches(order, cur->next->type)){
                // replace this one.
                Token* repl = init_token(cur->p->line);
                repl->subtoken_count = 2;
                repl->subtokens = init_token(cur->p->line);
                repl->subtokens = realloc_token(repl->subtokens, 2);
                repl->lnt = cur->next->type; // set operation
                memcpy(repl->subtokens, cur->p, sizeof(struct Token));
                memcpy(&repl->subtokens[1], cur->next->next->p, sizeof(struct Token));
                repl->type = T_ARITH;
                destroy_token(cur->p);
                destroy_token(cur->next->next->p);
                cur->p = repl;
                OpExpr* destroy = cur->next;
                cur->next = cur->next->next->next;
                if(cur->next != NULL){
                    cur->next->prev = cur;
                    // possible case. ok.
                }
                free(destroy);
                free(destroy->next);
                // ok.
            }
            else{
                cur = cur->next->next;
                // not precedence we are looking for.
            }
        }
    }
    // should be binary tree now.
    memcpy(e, head->p, sizeof(struct Token));
    destroy_token(head->p);
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
Lextoken* parse_body(Lextoken* p, Token* body, State* state){
    int ind = 0;
    body->subtokens = init_token(p->line);
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
                add_error(state, SYNTAXERROR, p->line, "failed to parse statement");
                return NULL; // statement with no term
            }
            
        }
        else if(match(o, TERM)){
            p = next(o); // empty stmt
        }
        else if(match(o, RIGHT_CRPAREN)){
            return o; // end of statements
        } else {
            add_error(state, SYNTAXERROR, p->line, "failed to parse statement");
        }
    }
}

 // funcdef = type, identifier, "(", [type, identifier], {"," type,  identifier}, ")", "{"
Lextoken* parse_funcdef(Lextoken* p, Token* def){
    def->type = T_FUNCDEF;
    def->subtokens = init_token(p->line);
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

// program = {[function] term }
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
            add_error(state, SYNTAXERROR, p->line, "failed to parse function definition or body");
            return NULL;
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

Token* init_token(int line){
    Token* t = malloc(sizeof(struct Token));
    t->str = NULL;
    t->subtoken_count = 0;
    t->lnt = 0;
    t->line = line;
    t->subtokens = NULL;
    t->chr = 0;
    return t;
}

Token* realloc_token(Token* ptr, int len){
    Token* pt = realloc(ptr, len*sizeof(struct Token));
    pt[len-1].str = NULL;
    pt[len-1].subtoken_count = 0;
    pt[len-1].lnt = 0;
    pt[len-1].line = ptr->line;
    pt[len-1].subtokens = NULL;
    pt[len-1].chr = 0;
    return pt;
}

void destroy_token(Token* ptr){
    return; // TODO make this work in the future.
    // note: dont recurse, functions already should do that
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
    if(p->type == T_ARITH){
        printf("%s%s <%c>\n", lvlstr, type(p), sym(p->lnt));
    }
    else if(p->str == NULL){
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
        case T_CARDINALITY: return "CARD";
        case T_ARITH: return "ARITH";
        default: return "UNKNOWN";
    }
    
}
