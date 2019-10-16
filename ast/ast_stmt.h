#ifndef __AST_STMT_H__
#define __AST_STMT_H__

#include"ast.h"
#include"core/common.h"

void* process_stmt(Token* t, Function* func, Scope* scope, Program* prog, State* state);

#endif
