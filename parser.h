#ifndef __PARSER_H__
#define __PARSER_H__

#include"lexer.h"

//token types


#define T_PROGRAM 1
#define T_FUNCTION 2
#define T_ASSIGNMENT 3
#define T_TYPE 4
#define T_VARIABLE 5
#define T_CHAR 7
#define T_INT 8
#define T_STRING 9
#define T_VARINIT 10
#define T_FUNCDEF 11
#define T_VARPARAM 12
#define T_BODY 13
#define T_RETURN 14
#define T_FUNCALL 15
#define T_INDGET 16
#define T_INDSET 17
#define T_ADDEQ 18
#define T_CARDINALITY 19

/**
 *
 * Token, nodes of a abstract syntax tree.
 * 
 * 
 */

typedef struct Token{
    int type;
    char* str;
    long lnt;
    char chr;
    int subtoken_count;
    struct Token* subtokens;
    
} Token;



int match(Lextoken* p, int m);
Lextoken* next(Lextoken* p);
Lextoken* parse_constant(Lextoken* p, Token* t);
Lextoken* parse_variable(Lextoken* p, Token* t);
Lextoken* parse_type(Lextoken* p, Token* t);
Lextoken* parse_expression(Lextoken* p, Token* t);
Lextoken* parse_varinit(Lextoken* p, Token* t);
Lextoken* parse_funcall(Lextoken* p, Token* t);
Lextoken* parse_arrind(Lextoken* p, Token* e);
Lextoken* parse_arrset(Lextoken* p, Token* e);
Lextoken* parse_addeq(Lextoken* p, Token* e);
Lextoken* parse_card(Lextoken* p, Token* e);
Lextoken* parse_assignment(Lextoken* p, Token* t);
Lextoken* parse_statement(Lextoken* p, Token* t);
Lextoken* parse_body(Lextoken* p, Token* t);
Lextoken* parse_funcdef(Lextoken* p, Token* t);
Lextoken* parse_function(Lextoken* p, Token* t);
Token* parse_program(Lextoken* p);
void print_tree(Token* p, int lvl);
char* type(Token* t);
Token* init_token();
Token* realloc_token(Token* ptr, int len);
void destroy_token(Token* ptr);
#endif
