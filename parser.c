#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"writer.h"
#include"common.h"
#include"error.h"
#include"lexer.h"
#include"parser.h"


// THIS IS THE FIFTH TIME THE PARSER HAS BEEN REWRITTEN. and hopefully the last.

/**
 * EBNF OF SYNTAX
 * 
 * 
 * 
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
Lextoken* parse_constant(Lextoken* p){
    int i = match(p, MINUS_SYM);
    i = i && match(next(p), INTEGER);
    if(!i){
        int i = match(p, CHAR);
        if(i){
            return next(p);
        }
    }
    else{
        return next(next(p));
    }
    return NULL;
}

// variable = identifier, ["[",expression,"]" ]
Lextoken* parse_variable(Lextoken* p){
    int i = match(p, IDENTIFIER);
    if(!i){
        return NULL;
    }
    int j = match(next(p), LEFT_SQPAREN);
    Lextoken* l = parse_expression(next(next(p)));
    j = j && (l != NULL);
    j = j && match(l, RIGHT_SQPAREN);
    if(j){
        return next(l);
    }
    else if(i){
        return next(p);
    }
    return NULL;
}

// type = indentifier, [ "[", "]"]
Lextoken* parse_type(Lextoken* p){
    int i = match(p, IDENTIFIER);
    if(!i){
        return NULL;
    }
    int j = match(next(p), LEFT_SQPAREN);
    j = j && match(next(next(p)), RIGHT_SQPAREN);
    if(j){
        return next(next(next(p)));
    }
    return next(p);
}

// expression = (constant | string | variable | funccall)
Lextoken* parse_expression(Lextoken* p){
    Lextoken* l = NULL;
    if((l = parse_variable(p))){
        return l;
    }
    if((l = parse_constant(p))){
        return l;
    }
    if(match(p, STRING)){
        return next(p);
    }
    return parse_funcall(p);
}

// varinit = type, identifier
Lextoken* parse_varinit(Lextoken* p){
    Lextoken* l = parse_type(p);
    if(match(l, IDENTIFIER)){
        return next(l);
    }
    return NULL;
}

// funccall = identifier, "(", [expression {",", expression } ] ")"
Lextoken* parse_funcall(Lextoken* p){
    int i = match(p, IDENTIFIER);
    i = i && match(next(p), LEFT_PAREN);
    if(!i){
        return NULL;
    }
    Lextoken* l = parse_expression(next(next(p)));
    if(l == NULL){
        i = i && match(next(next(p)), RIGHT_PAREN);
        if(i){
            return next(next(next(p)));
        }
        return NULL;
    }
    
    while(l != NULL){
        if(match(l, COMMA)){
            l = parse_expression(next(l));
            if(l == NULL){
                return NULL;
            }
        }
        else {
            if(match(l, RIGHT_PAREN)){
                return next(l);
            }
            return NULL;
        }
    }
    return NULL; // not possible (?)
}


// assignment = identifier, "=", expression
Lextoken* parse_assignment(Lextoken* p){
    int i = match(p, IDENTIFIER);
    i = i && match(next(p), EQUALS);
    Lextoken* l = parse_expression(next(next(p)));
    if(i && l != NULL){
        return l;
    }
    return NULL;
}

// statement = varinit | expression | assignment
Lextoken* parse_statement(Lextoken* p){
    Lextoken* l = parse_varinit(p);
    if(l != NULL){
        return l;
    }
    l = parse_expression(p);
    if(l != NULL){
        return l;
    }
    return parse_assignment(p);
}

// body = {[statement], term}
Lextoken* parse_body(Lextoken* p){
    while(1){
        Lextoken* o = p;
        p = parse_statement(p);
        if(p != NULL){
            if(match(p, TERM)){
                // good
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
Lextoken* parse_funcdef(Lextoken* p){
    Lextoken* l = parse_type(p);
    l = match(p, IDENTIFIER) ? next(p) : NULL;
    if((!match(l, LEFT_PAREN)) || (l == NULL)){
        return NULL;
    }
    Lextoken* ty1 = parse_type(l);
    Lextoken* ident1 = match(ty1, IDENTIFIER) ? next(ty1) : NULL;
    if(ident1 != NULL){
        l = ident1;
        while(1){
            if(!match(l, COMMA)){
                break;
            }
            Lextoken* ty = parse_type(next(l));
            Lextoken* ident = match(ty, IDENTIFIER) ? next(ty) : NULL;
            if(ident == NULL){
                return NULL;
            }
            l = ident;
        }
    }
    int j = match(l, RIGHT_PAREN);
    j = j && match(next(l), LEFT_CRPAREN);
    if(j){
        return next(next(l));
    }
    return NULL;
}

// function = funcdef, body, "}"
Lextoken* parse_function(Lextoken* p){
    Lextoken* l = parse_funcdef(p);
    l = parse_body(l);
    if(l != NULL && match(l, RIGHT_CRPAREN)){
        return next(l);
    }
    return NULL;
}

// program = {[function] term }
Lextoken* parse_program(Lextoken* p){
    while(1){
        Lextoken* l = parse_function(p);
        if(l != NULL){
            p = l;
        }
        if(!match(p, TERM)){
            return p;
        }
        p = next(p);
    }
}
