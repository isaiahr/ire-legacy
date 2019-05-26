#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include"parser.h"

#define S_ASSIGNMENT 1
#define S_CONSTANTASSIGNMENT 2
#define S_FUNCTIONCALL 3
#define S_VARINIT 4
#define S_RETURN 5
#define S_INDEX 6
#define S_INDEXEQUALS 7
#define S_ADDEQUALS 8

#define S_CONST_STRING 1
#define S_CONST_BYTE 2
#define S_CONST_INT 3

typedef struct Program {
    struct Function* funcs;
    int func_count;
    struct Type* types;
    int type_count;
} Program;

typedef struct Function {
    char* name;
    struct Type* retval;
    struct VarList* params;
    int param_count;
    struct VarList* vars;
    int var_count;
    struct Body* body;
    int native;
    
    // compile time
    int max_offset;
    char* write_name;
    int writ_return;
} Function;

typedef struct Body {
    struct Statement* stmt;
    struct Body* next;
} Body;

typedef struct VarList {
    struct Variable* var;
    struct VarList* next;
} VarList;

typedef struct Variable {
    struct Type* type;
    char* identifier;
    
    // compile time
    int offset;
} Variable; 

typedef struct Type {
    int width;
    char* identifier;
    struct Type** subtypes;
    int subtype_count;
    int* subtype_count_per;
} Type;

typedef struct Statement {
    void* stmt;
    int type;
} Statement;

typedef struct Assignment {
    struct Variable* from;
    struct Variable* to;
} Assignment;

typedef struct ConstantAssignment {
    char* string;
    char byte;
    long lnt;
    int type; // 1 = str, 2 = byte, 3 = int
    Variable* to;
} ConstantAssignment;

typedef struct FunctionCall {
    VarList* vars;
    int var_count;
    Function* func;
    Variable* to;
} FunctionCall;

typedef struct VarInit {
    Variable* var;
} VarInit;

typedef struct Index {
    Variable* arr;
    Variable* ind;
    Variable* to;
} Index;

typedef struct IndexEquals {
    Variable* arr;
    Variable* ind;
    Variable* eq;
} IndexEquals;

typedef struct AddEquals {
    Variable* var;
    Variable* delta;
} AddEquals;

typedef struct Return {
    Variable* var;
} Return;


Program* process_program(Token* t);
void process_function(Token* xd, Function* func, Program* prog);
void* process_stmt(Token* t, Function* func, Program* prog);
char* formatvar(Variable* var);
void print_func(Function* func);
void print_type(Type* t);
void print_prog(Program* prog);
Type* proc_type(char* ident, Program* prog);
Variable* mkvar(Function* func, Type* t);
Variable* mknvar(Function* func, char* str, Type* t);
Statement* mkinit(Variable* v);
Variable* proc_var(char* str, Function* func);
Function* proc_func(char* funcname, Program* prog);
char* clone(char* str);
void add_stmt_func(Statement* stmt, Function* func);

#endif
