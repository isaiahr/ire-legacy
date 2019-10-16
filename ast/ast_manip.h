#ifndef __AST_MANIP_H__
#define __AST_MANIP_H__

#include"ast.h"
#include"core/common.h"

VarList* add_varlist(VarList* vl, Variable* var);
Type* proc_type(char* ident, Program* prog);
Variable* mkvar(Function* func, Scope* scope, Type* t);
Statement* mkinit(Variable* var);
Variable* mknvar(Function* func, Scope* scope, char* str, Type* t);
Variable* proc_var(char* str, Scope* scope, Function* func);
Function* proc_func(char* funcname, Program* prog);
Type* arr_subtype(Type* arr, Program* p);
char* clone(char* str);
void add_stmt_func(Statement* stmt, Function* func, Scope* scope);
int verify_types(Type* want, ...);

#endif
