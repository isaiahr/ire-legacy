#ifndef __ERROR_H__
#define __ERROR_H__

#include"common/common.h"

#define LEXERROR 0
#define SYNTAXERROR 1
#define UNDEFVAR 2
#define UNDEFTYPE 6
#define UNDEFFUNC 3
#define DUPDEFFUNC 4
#define DUPDEFVAR 5
#define DUPDEFTYPE 7
#define INCOMPATTYPE 8

#define ERRORLEXING 1
#define ERRORPARSING 2
#define ERRORSEMANTIC 3

typedef struct Error{
    int type;
    char* msg;
    int count;
    struct Error* next;
} Error;

void add_error(State* state, int code, int line, char* msg);
void mark(State* state);

#endif
