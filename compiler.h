#ifndef __COMPILER_H__
#define __COMPILER_H__

extern void compile(State* state, Token* t);
extern Token* parsefile(State* state, char* data);
extern Token* join(Token* first, Token* second);
#endif
