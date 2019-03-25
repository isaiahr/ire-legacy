#ifndef __COMPILER_H__
#define __COMPILER_H__

extern void compile(State* state, char* data, long sz);
extern void process_token(Token* token, int line, State* state);

#endif
