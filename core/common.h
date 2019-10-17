#ifndef __IRE_COMMON_H__
#define __IRE_COMMON_H__

#include<stdio.h>
#define ENTRYFUNC "main"

typedef struct List{
    void* data;
    struct List* next;
} List;

//compiler state
typedef struct State{
    int comp_asm;
    int comp_llvm;
    int tempnum;
    int llvm;
    int lblcount;
    char* outputfile;
    int verbose;
    int annotate;
    int optimization;
    int writ_return;
    struct Error* errors;
    FILE* fp;
} State;

extern void debug(State* st, char* format, ...);
extern char* format(char* format, ...);
char sym(int id);

#endif
