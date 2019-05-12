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
    List* variables;
    List* functions;
    List* types;
    int comp_asm;
    int comp_llvm;
    int tempnum;
    int llvm;
    char* outputfile;
    int verbose;
    int annotate;
    Function* currentfunc;
    int writ_return;
    char* treg;
    char* tregm;
    FILE* fp;
} State;


/***
extern Variable* add_fakevar(Function* func, State* state);
extern Function* add_func(char* func, int defined, State* state);
extern Variable* add_var(Function* func, char* var, Type* type, State* state);
extern Type*     add_type(char* name, int id, State* state);
extern Type*     add_type_(char* name, int id, State* state);
extern Function* ref_func(char* func, State* state);
extern Variable* ref_var(Function* func, char* varn, State* state);
extern Type*     ref_type(char* type, State* state);
*/
#endif
