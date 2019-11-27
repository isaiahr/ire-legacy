#ifndef __PARSEUTILS_H__
#define __PARSEUTILS_H__

int match(Lextoken* p, int m);
Lextoken* next(Lextoken* p);
void print_tree(Token* p, int lvl);
char* type(Token* t);
Token* init_token();
void destroy_token(Token* ptr);
void destroy_children(Token* parent);
void destroy_youngest(Token* parent);
Token* allocate_child_token(Token* t, int line);
void adopt_child_token(Token* g, Token* c);
int subtoken_count(Token* t);
Token* subtoken(Token* parent, int index);

#endif
