#ifndef __LEXER_H__
#define __LEXER_H__

#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58)


#define LEXERROR -1
#define LEFT_PAREN 1
#define RIGHT_PAREN 2
#define LEFT_SQPAREN 3
#define RIGHT_SQPAREN 4
#define LEFT_CRPAREN 5
#define RIGHT_CRPAREN 6
#define INTEGER 7
#define CHAR 202
#define STRING 203
#define IDENTIFIER 10
#define TERM 11
#define MINUS_SYM 12
#define COMMA 13
#define EQUALS 14
#define EOF (-1)

typedef struct Lextoken{
    int line;
    int type;
    long lnt;
    char chr;
    char* str;
    struct Lextoken* next;
    struct Lextoken* prev;
} Lextoken;

extern Lextoken* lex(char* input);
extern Lextoken* lexone(char** i, int* line);
extern int digit(char input);
extern char* proc_str(char* data);
extern int beginswith(char* begin, char* str);

#endif
