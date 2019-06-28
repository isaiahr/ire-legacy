#ifndef __PARSER_H__
#define __PARSER_H__

#include"lexer.h"
#include"datastructs.h"

//token types


#define T_PROGRAM 1
#define T_FUNCTION 2
#define T_ASSIGNMENT 3
#define T_TYPE 4
#define T_VARIABLE 5
// 6 = ???
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
#define T_NEWARR 20
#define T_ARITH 21
#define T_TYPEDEF 22
#define T_TYPEVAL 23
#define T_ANDTYPE 24
#define T_ORTYPE 25
#define T_XORTYPE 26

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
    int line;
    struct Token* subtokens;
    
} Token;

Token* parse_program(Lextoken* p, State* state);


#endif
