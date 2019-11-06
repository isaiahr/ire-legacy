/**
 * ast.h
 * 
 **/

#ifndef __AST_H__
#define __AST_H__

#include"parser/parser.h"
#include"core/common.h"

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
#define S_INVERT 17
#define S_GETTAG 18

#define S_CONST_STRING 1
#define S_CONST_BYTE 2
#define S_CONST_INT 3
#define S_CONST_BOOLEAN 4

#define S_MODE_AND 1
#define S_MODE_OR 2
#define S_MODE_TYPE 3

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
    
    struct VarMetadata* meta;
    // compile time
    // asm loc
    int offset;
    int inited;
    // llvm loc
    int num;
} Variable;

typedef struct TagList {
    char* tag;
    Variable* var;
    // boolean
    int valid;
    struct TagList* next;
} TagList;

typedef struct VarMetadata {
    TagList* tags;
} VarMetadata;

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

typedef struct Invert{
    Variable* from;
    Variable* to;
} Invert;

typedef struct Arithmetic{
    Variable* left;
    Variable* right;
    Variable* to;
    int operation;
} Arithmetic;

/**
 * Typestructure is effectively a union of some things. it is a recursive type.
 * valid instance is: (sub, mode) | (sub, mode, next, segment) | (identifier, sbs, mode) | (identifier, sbs, next, segment, mode)
 * so either sub & mode exist or identifier and sbs exist. the former means we have children, combined according to mode. 
 * note mode will be set to S_MODE_TYPE if identifier and sbs exist.
 * if sub exists, then next type will have next and segment existing
 * also segment doesnt exists if parent is S_MODE_AND
 * 
 */
typedef struct TypeStructure {
    // descend
    struct TypeStructure* sub;
    // mode according to combined children
    int mode;
    // end of type.
    char* identifier;
    Type* sbs;
    // lateral
    struct TypeStructure* next;
    char* segment;
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
    Variable* src;
    int offsetptr;
} SetTag;

typedef struct GetTag{
    Variable* src;
    Variable* dest;
    int offsetptr;
} GetTag;

typedef struct Scope{
    VarList* vars;
    Body* body;
    struct Scope* parent;
    struct ScopeMetadata* meta;
    int offset;
} Scope;

typedef struct ScopeMetadata{
    TagList* tags;
} ScopeMetadata;

typedef struct IfStmt{
    // head = if, test nonnull = elseif, test null = else
    Variable* test;
    // scope for body of if
    Scope* scope;
    // scope for cond. this is nessecary to have a new scope to avoid sideeffects of
    // conditions. (short circuiting)
    Scope* cond;
    char* initlbl; // pre-condition test
    char* truelbl; // true cond -> branch here, code -> branch end.
    char* endlbl; // false cond -> branch elsestmt.initcond 
    // either elsestmt or endlbl exists.
    // if elsestmt exists, then endlbl = endlbl(elsestmt) (recursive)
    struct IfStmt* elsestmt;
} IfStmt;

Program* process_program(Token* t, State* state);


#endif
