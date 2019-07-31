#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include"parser.h"
#include"datastructs.h"

#define S_ASSIGNMENT 1
#define S_CONSTANTASSIGNMENT 2
#define S_FUNCTIONCALL 3
#define S_VARINIT 4
#define S_RETURN 5
#define S_INDEX 6
#define S_INDEXEQUALS 7
#define S_ADDEQUALS 8
#define S_CARDINALITY 9
#define S_NEWARRAY 10
#define S_ARITHMETIC 11
#define S_CONSTRUCTOR 12
#define S_ACCESSOR 13
#define S_SETMEMBER 14
#define S_SETTAG 15
#define S_IF 16

#define S_CONST_STRING 1
#define S_CONST_BYTE 2
#define S_CONST_INT 3

#define S_MODE_AND 1
#define S_MODE_OR 2
#define S_MODE_XOR 3
#define S_MODE_TYPE 4

typedef struct Program {
    struct Function* funcs;
    int func_count;
    struct TypeList* types;
    int type_count;
} Program;

typedef struct Function {
    char* name;
    struct Type* retval;
    struct VarList* params;
    int param_count;
    struct VarList* vars;
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

typedef struct TypeList {
    struct Type* type;
    struct TypeList* next;
} TypeList;

typedef struct Variable {
    struct Type* type;
    char* identifier;
    
    // compile time
    // asm loc
    int offset;
    int inited;
    // llvm loc
    int num;
} Variable; 

typedef struct Type {
    int width;
    char* identifier;
    char* llvm;
    // defined type <=> ts nonnull
    struct TypeStructure* ts;
    int internal_width; // for defined types
    // nonnull = array
    struct Type* array_subtype;
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

typedef struct Cardinality{
    Variable* from;
    Variable* to;
} Cardinality;

typedef struct NewArray{
    Variable* size;
    Variable* to;
} NewArray;

typedef struct Arithmetic{
    Variable* left;
    Variable* right;
    Variable* to;
    int operation;
} Arithmetic;

typedef struct TypeStructure {
    // descend
    struct TypeStructure* sub;
    // or
    char* identifier;
    Type* sbs;
    // lateral
    struct TypeStructure* next;
    char* segment;
    int mode;
} TypeStructure;

typedef struct Constructor {
    Type* type;
    Variable* to;
} Constructor;

typedef struct Accessor {
    Variable* src;
    int offsetptr;
    Variable* to;
} Accessor;

typedef struct Setmember{
    Variable* dest;
    int offsetptr;
    Variable* from;
} Setmember;

typedef struct SetTag{
    Variable* dest;
    int offsetptr;
} SetTag;

typedef struct Scope{
    VarList* vars;
    Body* body;
    struct Scope* parent;
    int offset;
} Scope;

typedef struct IfStmt{
    Variable* test;
    Scope* scope;
    char* truelbl;
    char* endlbl;
} IfStmt;

Program* process_program(Token* t, State* state);


#endif
