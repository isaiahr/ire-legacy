#ifndef __PARSER_H__
#define __PARSER_H__

//token types


#define ASSIGNMENT 1
#define FUNCTION_CALL 2
#define FUNCTION_DEFN 3
#define FUNCTION_RETURN 4
#define FUNCTION_END 5
#define COMMENT 6
#define VARIABLE_DEFN 7
#define VARIABLE_REF 8
#define IMPORT 9
#define INT 10
#define CHAR 202
#define CONDITIONAL 12
#define ASM 13
#define ARRAY_ADD 14
#define ARRAY_INDEX 15
#define ARRAY_SET 16
#define STRING 203
#define INVALID -1
// Data structures


typedef struct Token{
    int type;
    Type* t;
    Variable* var1;
    Variable* var2;
    Function* func;
    char* str;
    char chr;
    long nt;
    struct Token* t1;
    struct Token* t2;
    struct Token* list;
} Token;

int match(Lextoken* p, int m);
Lextoken* next(Lextoken* p);
Lextoken* parse_constant(Lextoken* p);
Lextoken* parse_variable(Lextoken* p);
Lextoken* parse_type(Lextoken* p);
Lextoken* parse_expression(Lextoken* p);
Lextoken* parse_varinit(Lextoken* p);
Lextoken* parse_funcall(Lextoken* p);
Lextoken* parse_assignment(Lextoken* p);
Lextoken* parse_statement(Lextoken* p);
Lextoken* parse_body(Lextoken* p);
Lextoken* parse_funcdef(Lextoken* p);
Lextoken* parse_function(Lextoken* p);
Lextoken* parse_program(Lextoken* p);
#endif
