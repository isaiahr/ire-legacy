#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include"parser.h"

#define S_ASSIGNMENT 1
#define S_CONSTANTASSIGNMENT 2
#define S_FUNCTIONCALL 3
#define S_VARINIT 4
#define S_RETURN 5

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
    struct Variable* params;
    int param_count;
    struct Variable* vars;
    int var_count;
    struct Body* body;
} Function;

typedef struct Body {
    struct Statement* stmt;
    struct Body* next;
} Body;

typedef struct Variable {
    struct Type* type;
    char* identifier;
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
    Variable* vars;
    int var_count;
    Function* func;
    Variable* to;
} FunctionCall;

typedef struct VarInit {
    Variable* var;
} VarInit;

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
Variable* mkvar(Type* t);
Variable* mknvar(Function* func, char* str, Type* t);
Variable* proc_var(char* str, Function* func);
Function* proc_func(char* funcname, Program* prog);
char* clone(char* str);
void add_stmt_func(Statement* stmt, Function* func);

#endif
