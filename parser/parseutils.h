#ifndef __PARSEUTILS_H__
#define __PARSEUTILS_H__

int match(Lextoken* p, int m);
Lextoken* next(Lextoken* p);
void print_tree(Token* p, int lvl);
char* type(Token* t);
Token* init_token();
Token* realloc_token(Token* ptr, int len);
void destroy_token(Token* ptr);

#endif
