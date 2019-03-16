#ifndef __DATASTRUCTS_H__
#define __DATASTRUCTS_H__
#define ENTRYFUNC "main"
#define VARTYPE_INTEGER 301
#define VARTYPE_STRING 302

//Data structures
typedef struct Function{
    char* name;
    char* write_name;
    int max_offset;
    int defined;
}Function;

typedef struct Variable{
    char* name;
    int offset;
    Function* func;
    int type;
    char* write_name;
}Variable;

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
    Function* currentfunc;
    int writ_return;
    FILE* fp;
} State;

extern Function* add_func(char* func, int defined, State* state);
extern Variable* add_var(char* func, char* var, int type, State* state);
extern Function* ref_func(char* func, State* state);
extern Variable* ref_var(char* func, char* varn, State* state);

#endif
