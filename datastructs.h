#ifndef __DATASTRUCTS_H__
#define __DATASTRUCTS_H__
#define ENTRYFUNC "main"
//Data structures

#include"semantic.h"

//linked list
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
    char* outputfile;
    int verbose;
    int annotate;
    int writ_return;
    FILE* fp;
} State;


#endif
