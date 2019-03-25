#ifndef __PARSER_H__
#define __PARSER_H__

#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58) 

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
#define IMMEDIATE 10
#define CONDITIONAL 11
#define ASM 12
#define INVALID -1
// Data structures


typedef struct Token{
    int type;
    Type* t;
    Variable* var1;
    Variable* var2;
    Function* func;
    char* str;
    long nt;
    struct Token* t1;
    struct Token* t2;
} Token;

extern Token* tokenize(char* str, int line, State* state);
extern int indexchr(char* str, char chr);
extern void validname(char* name, int line);
extern char* match_paren(char* input);
extern char* copy(char* token, char* pass, char* end);
extern char* oldcopy(char* token, int ind0, int ind1);
extern int beginswith(char* begin, char* token);

#endif
