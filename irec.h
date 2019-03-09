#ifndef __IREC_H__
#define __IREC_H__
#define BADUSAGE 1
#define NOFILE -1

//Data structures

//linked list
typedef struct List{
    void* data;
    struct List* next;
} List;

//compiler state
typedef struct State{
    List* variables;
    List* functions;
    int comp_asm;
    char* outputfile;
    int verbose;
    int annotate;
    FILE* fp;
} State;

#endif
