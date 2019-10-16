#ifndef __AST_TYPES__H_
#define __AST_TYPES__H_

#include"ast.h"
#include"core/common.h"

void write_structure(TypeStructure* write, Token* src, Program* prog, State* state);
int bytes(TypeStructure* ts);
Type* findtype(Type* t, char* ident);
char* duptypes(Type* t);
int findoffset(Type* t, char* ident);
int findoffsettag(Type* t, char* ident);

#endif
