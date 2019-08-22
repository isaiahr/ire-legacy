#ifndef __LEXER_H__
#define __LEXER_H__

#include"common/common.h"

#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58)


#define LEXERROR 0
#define LEFT_PAREN 1
#define RIGHT_PAREN 2
#define LEFT_SQPAREN 3
#define RIGHT_SQPAREN 4
#define LEFT_CRPAREN 5
#define RIGHT_CRPAREN 6
#define INTEGER 7
#define LCHAR 8
#define LSTRING 9
#define IDENTIFIER 10
#define TERM 11
// 12 = ??
#define COMMA 13
#define EQUALS 14
#define LEOF 15
#define RETURN 16
#define ADDEQ 17
#include"common/operators.h" // MAKE SURE THESE MATCH UP ( no duplicates). compiler should warn if incorrect
#define NEW 25
#define VOID 26
#define TYPE 27
#define COLON 28
#define DOT 29
#define IF 30
#define TRUE 31
#define FALSE 32
#define DOUBLEPIPE 33
#define EXCLAMATION 34

typedef struct Lextoken{
    int line;
    int type;
    long lnt;
    char chr;
    char* str;
    struct Lextoken* next;
    struct Lextoken* prev;
} Lextoken;

extern Lextoken* lex(char* input, State* state);
extern Lextoken* lexone(char** i, int* line);
extern int digit(char input);
extern char* proc_str(char* data, char** adv);
extern int beginswith(char* begin, char* str);

#endif
